#pragma once
#include <filesystem>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include <limits>
// #include <map>

#include "Common.h"

#include "../../MatrImg.h"
#include "../../CachedBarcode.h"
#include "../../Clusterizers/ConverctItem.h"

class DatasetWork
{
	std::unordered_map<BackPathStr, int> sourceFiles;
	int maxAllowed;
	int NC = 2;
public:

	void open()
	{
		BackPathStr filesRoot = "/Users/sam/Edu/datasets/hirise-map-proj-v3/map-proj-v3";
		NC = 7;
		std::ifstream srcleab("/Users/sam/Edu/datasets/hirise-map-proj-v3/labels-map-proj-v3.txt");
		// Read file line by line

		std::string line;
		std::vector<int> counter(NC, 0);

		while (std::getline(srcleab, line))
		{
			int p = line.find_last_of(' ');
			std::string name = line.substr(0, p);
			int id = std::stoi(line.substr(p + 1));
			counter[id]++;
			sourceFiles.insert(std::pair(filesRoot / name, id));
		}

		std::cout << "Total: " << std::accumulate(counter.begin(), counter.end(), 0) << std::endl;
		for (size_t i = 0; i < NC; i++)
		{
			std::cout << i + 1 << ": " << counter[i] << std::endl;
		}

		maxAllowed = *std::min_element(counter.begin(), counter.end());
	}

	void openCraters(BackStringView name)
	{
		NC = 2;
		BackPathStr datasets("/Users/sam/Edu/datasets");
		BackPathStr srcleab(datasets / name / "train/crater");
		std::vector<int> counter(NC, 0);
		// iterate over each file in the directory
		for (const auto& entry : std::filesystem::directory_iterator(srcleab))
		{
			// std::cout << entry.path() << std::endl;
			sourceFiles.insert(std::pair<std::string, int>(entry.path(), 0));
			counter[0]++;
		}

		srcleab =datasets / name / "train/noncrater";
		for (const auto& entry : std::filesystem::directory_iterator(srcleab))
		{
			sourceFiles.insert(std::pair<std::string, int>(entry.path(), 1));
			counter[1]++;
		}

		std::cout << "Total: " << std::accumulate(counter.begin(), counter.end(), 0) << std::endl;
		for (size_t i = 0; i < NC; i++)
		{
			std::cout << i + 1 << ": " << counter[i] << std::endl;
		}
	}

	template<class C>
	void predict(int maxAllowed, ConvertCollection& landscape, const bc::barstruct& constr, C& processor) const
	{
		processor.setClasses(NC);
		std::cout << "Sample " << maxAllowed << " elements from each cluster" << std::endl;

		std::vector<int> counter(NC, 0);
		int added = 0;

		std::vector<BackString> names;
		for (auto& entry : sourceFiles)
		{
			int correctId = entry.second;
			if (counter[correctId] >= maxAllowed)
				continue;

			// // // Skip list
			// switch (correctId)
			// {
			// case 0:
			// case 1:
			// case 2:
			// // case 3:
			// case 4:
			// case 5:
			// // case 6:
			// case 7:
			// 	continue;
			// }

			BackPathStr path =  entry.first;
			// assert(pathExists(path));

			counter[correctId]++;
			BackImage main = imread(path);

			CachedBaritemHolder cache;
			cache.create(&main, constr, nullptr);

			landscape.addAllLines(cache);
			ConvertClass* item = landscape.back();

			// Fitting --------------- ---------------
			added += processor.addToSet(item, correctId);
			// --------------- --------------- ---------------

			names.push_back(entry.first.filename().string());
		}

		processor.predict();
		int results[1000];
		std::fill_n(results, 1000, 0);

		int correctCount = 0;
		int maxPred = 0;
		for (size_t i = 0; i < added; i++)
		{
			int prediction = processor.test(i);
			// int correctId = sourceFiles[names[i]];
			// bool correct = prediction == correctId;
			// if (correct)
			// {
			// 	results[correctId]++;
			// 	correctCount++;
			// }
			// assert(prediction < 7);
			results[prediction]++;
			if (prediction > maxPred)
				maxPred = prediction;

			// std::cout << paths[i] << " -> " << cluster.test(i);
			// if (correct)
			// {
			// 	std::cout << " (correct)";
			// }
			// else
			// {
			// 	std::cout << " (incorrect, " << prediction << " vs " << correctId << ")";
			// }
			// std::cout << std::endl;
		}

		// std::cout << "Correct: " << correctCount << "/" << added << " (" << correctCount * 100.0 / added << "%)" << std::endl;
		correctCount = 0;
		float res = 0;
		for (size_t i = 0; i <= maxPred; i++)
		{
			int r = results[i];

			if (i >= NC)
			{
				std::cout << i + 1 << ": " << r << "/?" << std::endl;
				continue;
			}

			int c = counter[i];

			int t;
			if (r <= c)
				t = c - r;
			else
				t = -(r - c);

			correctCount += t;

			int a = float(r) / float(c);
			if (a >= 0)
				a = 1.0 - a;

			std::cout << i + 1 << ": " << r << "/" << c << std::endl;
			res += a;
		}
		std::cout << "Correct: " << correctCount << "/" << added << " (" << res * 100.f << "%)" << std::endl;
	}
};

enum class SignatureType
{
	Iter,
	Combined,
	CombinedIter
};

template<class T, SignatureType type>
class SignatureProcessor
{
	T dummy;
public:
	SignatureProcessor(T& ref) : ref(ref) {}
	SignatureProcessor() : ref(dummy) {}

	int addToSet(ConvertClass* item, int classId)
	{
		hist.push_back({});
		switch (type)
		{
		case SignatureType::Iter:
			item->getSignatureAsVector(hist.back());
			break;
		case SignatureType::Combined:
			item->getCombinedPointsAsHist(hist.back());
			break;
		case SignatureType::CombinedIter:
			item->getCombinedPointsAsSignature(hist.back());
			break;
		}
		return 1;
	}
	void setClasses(int n)
	{
		ref.setClasses(n);
	}

	void predict()
	{
		ref.predict(hist);
	}

	int test(int id)
	{
		return ref.test(id);
	}

private:
	T& ref;
	std::vector<std::vector<float>> hist;
};

template<class T>
class PointsProcessor
{
	T dummy;
public:
	PointsProcessor(T& ref) : ref(ref) {}
	PointsProcessor() : ref(dummy) {}

	int addToSet(ConvertClass* item, int classId)
	{
		landspacePoints.push_back({});
		item->getCombinedPoints(landspacePoints.back());
		return 1;
	}

	void setClasses(int n)
	{
		ref.setClasses(n);
	}

	void predict()
	{
		ref.predict(landspacePoints);
	}

	int test(int id)
	{
		return ref.test(id);
	}

private:
	T& ref;
	std::vector<std::vector<landres>> landspacePoints;
};
