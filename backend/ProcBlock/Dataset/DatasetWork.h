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

constexpr int NC = 2;
class DatasetWork
{
	std::unordered_map<std::string, int> sourceFiles;
	int maxAllowed;
public:

	void open()
	{
		std::ifstream srcleab("/Users/sam/Edu/datasets/hirise-map-proj-v3/labels-map-proj-v3.txt");
		// Read file line by line

		std::string line;
		int counter[NC];
		std::fill_n(counter, NC, 0);

		while (std::getline(srcleab, line))
		{
			int p = line.find_last_of(' ');
			std::string name = line.substr(0, p);
			int id = std::stoi(line.substr(p + 1));
			counter[id]++;
			sourceFiles.insert(std::pair(name, id));
		}

		std::cout << "Total: " << std::accumulate(counter, counter + NC, 0) << std::endl;
		for (size_t i = 0; i < NC; i++)
		{
			std::cout << i + 1 << ": " << counter[i] << std::endl;
		}

		maxAllowed = *std::min_element(counter, counter + NC);
	}

	template<class C>
	void predict(BackPathStr filesRoot, int maxAllowed, ConvertCollection& landscape, const bc::barstruct& constr, C& processor) const
	{
		std::cout << "Sample " << maxAllowed << " elements from each cluster" << std::endl;

		int counter[NC];
		std::fill_n(counter, NC, 0);
		int added = 0;

		std::vector<BackString> names;
		for (auto& entry : sourceFiles)
		{
			int correctId = entry.second;
			if (counter[correctId] >= maxAllowed)
				continue;

			// // Skip list
			switch (correctId)
			{
			case 0:
			case 1:
			case 2:
			// case 3:
			case 4:
			case 5:
			// case 6:
			case 7:
				continue;
			}

			BackString path = filesRoot / entry.first;
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

			names.push_back(entry.first);
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
				t = std::max(c - r, 0);

			int a = (c - t);
			if (a >= 0)
				correctCount += a;

			std::cout << i + 1 << ": " << r << "/" << c << std::endl;
			res += t * 100.0 / (c);
		}
		std::cout << "Correct: " << correctCount << "/" << added << " (" << res / NC << "%)" << std::endl;
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
