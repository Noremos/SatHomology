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
#include "../../Clusterizers/LandscapeItem.h"

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

	void openCraters(BackStringView name, BackStringView name1 = "crater", BackStringView name2 = "noncrater")
	{
		NC = 2;
		BackPathStr datasets("/Users/sam/Edu/datasets");
		BackPathStr srcleab(datasets / name / name1);
		std::vector<int> counter(NC, 0);
		// iterate over each file in the directory
		for (const auto& entry : std::filesystem::directory_iterator(srcleab))
		{
			// std::cout << entry.path() << std::endl;
			sourceFiles.insert(std::pair<std::string, int>(entry.path(), 0));
			counter[0]++;
		}

		srcleab =datasets / name / name2;
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
	void predict(int maxAllowed, LandscapeCollection& landscape, const bc::barstruct& constr, C& processor) const
	{
		landscape.clear();
		processor.setClasses(NC);
		std::cout << "Sample " << maxAllowed << " elements from each cluster" << std::endl;

		std::vector<int> counter(NC, 0);
		int added = 0;

		std::vector<int> correctIds;
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
			LandscapeClass* item = landscape.back();

			// Fitting --------------- ---------------
			added += processor.addToSet(item, correctId);
			// --------------- --------------- ---------------

			// names.push_back(entry.first.filename());
			correctIds.push_back(correctId);
		}

		processor.predict();
		int results[1000];
		std::fill_n(results, 1000, 0);

		int fals[1000];
		std::fill_n(fals, 1000, 0);

		int correctCount = 0;
		int maxPred = 0;
		for (size_t i = 0; i < added; i++)
		{
			int prediction = processor.test(i);
			int correctId = correctIds[i];
			bool correct = prediction == correctId;
			if (correct)
			{
				results[correctId]++;
				correctCount++;
			}
			else
				fals[correctId]++;
			// assert(prediction < 7);
			// results[prediction]++;
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
		float res = 0;
		correctCount = 0;
		for (size_t i = 0; i <= maxPred; i++)
		{
			int r = results[i];

			if (i >= NC)
			{
				std::cout << i + 1 << ": " << r << "/?" << std::endl;
				correctCount -= r;
				continue;
			}

			int c = counter[NC - i - 1];

			int t = c - r;
			correctCount += t;

			float a = float(r) / float(c);
			if (a >= 0)
				a = 1.0 - a;

			std::cout << i + 1 << ": " << r << "/" << c << " (false is " << fals[NC-i-1] << ")" << std::endl;
			res += a;
		}
		std::cout << "Correct: " << correctCount << "/" << added << " (" << res * 100.f << "%)" << std::endl;
	}
};

enum class SignatureType
{
	Iter,
	Combined,
	CombinedIter,
	SupFull
};

template<class T, SignatureType type>
class SignatureProcessor
{
	T dummy;
public:
	SignatureProcessor(T& ref, int resolution) : ref(ref), resolution(resolution) {}
	SignatureProcessor(int resolution) : ref(dummy), resolution(resolution) {}

	int addToSet(LandscapeClass* item, int classId)
	{
		landscapes.push_back({});
		switch (type)
		{
		case SignatureType::Iter:
			landscapes.back().push_back({});
			item->getSignatureAsVector(landscapes.back().back().points, resolution);
			break;
		case SignatureType::Combined:
			landscapes.back().push_back({});
			item->getCombinedPointsAsHist(landscapes.back().back().points, resolution);
			break;
		case SignatureType::CombinedIter:
			landscapes.back().push_back({});
			item->getCombinedPointsAsSignature(landscapes.back().back().points, resolution);
			break;
		case SignatureType::SupFull:
			landscapes.push_back(item->getIterLandscape(resolution));
			break;
		}
		return 1;
	}

	int resolution = 1;
	void setClasses(int n)
	{
		ref.setClasses(n);
	}

	void predict()
	{
		ref.predict(landscapes);
	}

	int test(int id)
	{
		return ref.test(id);
	}

private:
	T& ref;
	std::vector<IterLandscape> landscapes;
};

template<class T>
class PointsProcessor
{
	T dummy;
public:
	PointsProcessor(T& ref) : ref(ref) {}
	PointsProcessor() : ref(dummy) {}

	int addToSet(LandscapeClass* item, int classId)
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
	std::vector<std::vector<LandPoint>> landspacePoints;
};
