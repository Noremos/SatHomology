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
	std::unordered_map<std::string, int> sourceFiles;
	int maxAllowed;
public:

	void open()
	{
		std::ifstream srcleab("/Users/sam/Edu/datasets/hirise-map-proj-v3/labels-map-proj-v3.txt");
		// Read file line by line

		std::string line;
		int counter[7] = { 0, 0, 0, 0, 0, 0, 0 };

		while (std::getline(srcleab, line))
		{
			int p = line.find_last_of(' ');
			std::string name = line.substr(0, p);
			int id = std::stoi(line.substr(p + 1));
			counter[id]++;
			sourceFiles.insert(std::pair(name, id));
		}

		std::cout << "Total: " << std::accumulate(counter, counter + 7, 0) << std::endl;
		for (size_t i = 0; i < 7; i++)
		{
			std::cout << i + 1 << ": " << counter[i] << std::endl;
		}

		maxAllowed = *std::min_element(counter, counter + 7);
	}

	template<class C>
	void predict(BackPathStr filesRoot, int maxAllowed, ConvertCollection& landscape, const bc::barstruct& constr, C& processor) const
	{
		std::cout << "Sample " << maxAllowed << " elements from each cluster" << std::endl;

		int counter[7] = { 0, 0, 0, 0, 0, 0, 0 };
		int added = 0;

		std::vector<BackString> names;
		for (auto& entry : sourceFiles)
		{
			int correctId = entry.second;
			if (counter[correctId] > maxAllowed)
				continue;

			// Skip list
			// switch (correctId)
			// {
			// case 1:
			// case 2:
			// case 5:
			// case 6:
			// 	continue;
			// }

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
		int results[7] = { 0, 0, 0, 0, 0, 0, 0 };
		std::fill_n(results, 7, 0);

		int correctCount = 0;
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
			assert(prediction < 7);
			results[prediction]++;

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
		for (size_t i = 0; i < 7; i++)
		{
			int r = results[i];
			int c = counter[i];
			if (r > c)
				r = std::max(c - r, 0);

			int a = (c - r);
			if (a >= 0)
				correctCount += a;

			std::cout << i + 1 << ": " << r << "/" << c << std::endl;
			res += r * 100.0 / (c);
		}
		std::cout << "Correct: " << correctCount << "/" << added << " (" << res / 7 << "%)" << std::endl;
	}
};

template<class T>
class StrictIterProcessor
{
	T dummy;
public:
	StrictIterProcessor(T& ref) : ref(ref) {}
	StrictIterProcessor() : ref(dummy) {}

	int addToSet(ConvertClass* item, int classId)
	{
		landspace.push_back({});
		item->getCombinedPointsAsSignature(landspace.back());
		return 1;
	}

	void predict()
	{
		ref.predict(landspace);
	}

	int test(int id)
	{
		return ref.test(id);
	}

private:
	T& ref;
	std::vector<std::vector<float>> landspace;
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

template<class T>
class IterProcessor
{
	T dummy;
public:
	IterProcessor(T& ref) : ref(ref) {}
	IterProcessor() : ref(dummy) {}

	int addToSet(ConvertClass* item, int classId)
	{
		landspacePoints.push_back({});
		item->getSignatureAsVector(landspacePoints.back());
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
	std::vector<std::vector<float>> landspacePoints;
};
