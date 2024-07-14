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
#include "Hungarian.h" // Подключаем библиотеку для Венгерского алгоритма (lap.h)

class BarWriter
{

};

class DatasetWork
{
	std::unordered_map<BackPathStr, int> sourceFiles;
	int NC = 2;
public:

	void open(int maxAllowed)
	{
		BackPathStr filesRoot = "/Users/sam/Edu/datasets/objects/hirise-map-proj-v3";
		BackPathStr images = filesRoot / "map-proj-v3";
		NC = 8;
		std::ifstream srcleab(filesRoot / "labels-map-proj-v3.txt");
		// Read file line by line

		std::string line;
		std::vector<int> counter(NC, 0);
		rnames.resize(NC);

		while (std::getline(srcleab, line))
		{
			int p = line.find_last_of(' ');
			std::string name = line.substr(0, p);
			int id = std::stoi(line.substr(p + 1));
			counter[id]++;
			if (counter[id] >= maxAllowed)
				continue;;

			sourceFiles.insert(std::pair(images / name, id));
		}

		std::cout << "Total: " << std::accumulate(counter.begin(), counter.end(), 0) << std::endl;
		for (size_t i = 0; i < NC; i++)
		{
			std::cout << i + 1 << ": " << counter[i] << std::endl;
		}

		maxAllowed = *std::min_element(counter.begin(), counter.end());
	}

	std::vector<BackString> rnames;

	void openCraters(int maxAllowed, BackStringView name, const std::vector<BackString>& names = {})
	{
		BackPathStr datasets("/Users/sam/Edu/datasets");
		BackPathStr srcleab(datasets / name);

		if (names.size() == 0)
		{
			std::vector<BackString> newNames;
			for (const auto& entry : std::filesystem::directory_iterator(srcleab))
			{
				BackString name = entry.path().filename().string();
				if (name.starts_with('.'))
					continue;

				newNames.push_back(entry.path().filename().string());
			}
			if (newNames.size() == 0)
				return;

			openCraters(maxAllowed, name, newNames);
			return;
		}

		rnames = names;
		NC = names.size();

		std::vector<int> counter(NC, 0);

		int nc = 0;
		// iterate over each name
		for (auto& className : names)
		{
			// iterate over each file in the directory
			BackPathStr srcleanb(srcleab / className);
			for (const auto& entry : std::filesystem::directory_iterator(srcleanb))
			{
				// std::cout << entry.path() << std::endl;
				sourceFiles.insert(std::pair<std::string, int>(entry.path(), nc));
				counter[nc]++;
				if (counter[nc] >= maxAllowed)
					break;
			}

			nc++;
		}

		std::cout << "Total: " << std::accumulate(counter.begin(), counter.end(), 0) << std::endl;
		for (size_t i = 0; i < NC; i++)
		{
			std::cout << i + 1 << ": " << counter[i] << std::endl;
		}

	}

	int added = 0;
	std::vector<int> totalAdded;
	std::vector<int> correctIds;

	template<class C>
	void collect(int maxAllowed, LandscapeCollection& landscape, const bc::barstruct& constr, C& processor)
	{
		landscape.clear();
		processor.clear();
		processor.setClasses(NC);
		std::cout << "Sample " << maxAllowed << " elements from each cluster" << std::endl;

		added = 0;
		totalAdded.clear();
		totalAdded.resize(NC, 0);

		// StateBinFile::BinStateWriter writer;
		// writer.open("binitem.bin");


		correctIds = {};
		for (auto& entry : sourceFiles)
		{
			int correctId = entry.second;

			BackPathStr path =  entry.first;
			// assert(pathExists(path));

			totalAdded[correctId]++;
			BackImage main = imread(path);

			CachedBaritemHolder cache;
			cache.create(&main, constr, nullptr);
			// cache.saveLoadState(&writer);

			landscape.addAllLines(cache);
			LandscapeClass* item = landscape.back();

			// Fitting --------------- ---------------

			if constexpr(std::is_base_of<BarWriter, C>::value)
			{
				processor.add(cache, correctId);
			}
			added += processor.addToSet(item, correctId);
			if (added % 20 == 0)
				cout << added << "/" << sourceFiles.size() << endl;
			// --------------- --------------- ---------------

			// names.push_back(entry.first.filename());
			correctIds.push_back(correctId);
		}

	}

	template<class C>
	std::pair<int, int> predict(C& processor) const
	{
		std::cout << "Predicting..." << std::endl;
		processor.predict();
		if (added == 0)
			return {0, 0};

		int corrcts[1000];
		std::fill_n(corrcts, 1000, 0);

		int fals[1000];
		std::fill_n(fals, 1000, 0);

		int correctCount = 0;
		std::vector<int> predictions;
		for (size_t i = 0; i < added; i++)
		{
			predictions.push_back(processor.test(i));
		}

		std::vector<int> tr = mapIds(correctIds, predictions, NC, true);
		// std::cout << "Correct: " << correctCount << "/" << added << " (" << correctCount * 100.0 / added << "%)" << std::endl;

		int maxPred = 0;
		assert(tr.size() == predictions.size());
		for (size_t i = 0; i < predictions.size(); i++)
		{
			int r = tr[i];
			if (r == -1)
				continue;

			assert(r < NC);
			{
				// fals[r]++;
				// maxPred = std::max(maxPred, r);
				// continue;
			}

			if (r == correctIds[i])
			{
				corrcts[r]++;
				correctCount++;
			}
			else
			{
				// correctCount--;
				fals[r]++;
			}
		}

		float res = 0;
		for (size_t i = 0; i < NC; i++)
		{
			int correct = corrcts[i];
			int total = totalAdded[i];

			std::cout << rnames[i] << " (" << i << "): " << correct << "/" << total << " (false is " << fals[i] << ")" << std::endl;
		}

		res = correctCount * 1.0 / added;
		std::cout << "====================================" << std::endl;
		std::cout << "----------> Total Correct: " << correctCount << "/" << added << " (" << res * 100.f << "%)" << std::endl;
		std::cout << "====================================" << std::endl << std::endl;
		// return procent
		return {correctCount,added};
	}


	static void printConfusionMatrix(const vector<vector<double>>& cm)
	{
		using namespace std;
		for (const auto& row : cm)
		{
			for (int val : row) {
				cout << val << " ";
			}
			cout << endl;
		}
	}

	static void createConfusionMatrix(const std::vector<int>& true_labels, const std::vector<int>& predicted_labels, std::vector<std::vector<double>>& cm, int num_classes) {
		cm.resize(num_classes, std::vector<double>(num_classes, 0));
		for (size_t i = 0; i < true_labels.size(); ++i)
		{
			assert(true_labels[i] < num_classes);
			assert(predicted_labels[i] < num_classes);
			cm[true_labels[i]][predicted_labels[i]]++;
		}
	}

	static std::vector<int> mapIds(std::vector<int> true_labels, std::vector<int> predicted_labels, int num_classes, bool printMap = false)
	{
		assert(true_labels.size() == predicted_labels.size());
		using namespace std;

		vector<vector<double>> confusion_matrix;
		createConfusionMatrix(true_labels, predicted_labels, confusion_matrix, num_classes);
		// cout << "Confusion Matrix:" << endl;
		// printConfusionMatrix(confusion_matrix);

		// Hungarian algorithm requires cost matrix where we minimize the cost
		// Since we're maximizing the match between true_labels and predicted_labels,
		// we use negative values in the cost matrix
		std::vector<std::vector<double>> cost_matrix(num_classes, std::vector<double>(num_classes));
		for (int i = 0; i < num_classes; ++i)
		{
			for (int j = 0; j < num_classes; ++j)
			{
				cost_matrix[i][j] = -confusion_matrix[i][j];
				// // REMOVE IF BREAKS
				// if (cost_matrix[i][j] < 0)
				// 	cost_matrix[i][j] = 0;
			}
		}

		HungarianAlgorithm alg;
		vector<int> assignment;
		double cost = alg.Solve(cost_matrix, assignment);


		std::unordered_map<int, int> label_mapping;
		for (int i = 0; i < num_classes; ++i) {
			label_mapping[assignment[i]] = i ;
		}

		if (printMap)
		{
			std::cout << "Mapping (original to new):" << std::endl;
			for (const auto& pair : label_mapping) {
				std::cout << pair.first << " -> " << pair.second << std::endl;
			}
		}

		// Apply the new mapping to predicted labels
		std::vector<int> new_predicted_labels(predicted_labels.size());
		for (size_t i = 0; i < predicted_labels.size(); ++i) {
			new_predicted_labels[i] = label_mapping[predicted_labels[i]];
		}

		// std::cout << "New Predicted Labels:" << std::endl;
		// for (int label : new_predicted_labels) {
		// 	std::cout << label << " ";
		// }
		// std::cout << std::endl;

		return new_predicted_labels;
	}
};

enum class SignatureType
{
	Iter,
	Combined,
	CombinedIter,
	SupFull
};



template<class T>
class PointProcessor
{
public:
	T dummy;

	PointProcessor(T& ref) : ref(ref) {}
	PointProcessor() : ref(dummy) {}

	void clear()
	{
		landscapes.clear();
	}
	int addToSet(LandscapeClass* item, int classId)
	{
		landscapes.push_back(item->landscape);
		return 1;
	}

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
	std::vector<Landscape> landscapes;
};




class ClassProcessor
{
	std::vector<std::pair<Landscape,int>> landscapes;
	StateBinFile::BinStateWriter writer;
public:
	ClassProcessor()
	{
		writer.open("out.bin");
	}

	bool classMode = false;
	void switchToClassMode()
	{
		classMode = true;
	}
	std::vector<int> out;

	int addToSet(LandscapeClass* item, int classId)
	{
		if (!classMode)
		{
			landscapes.push_back({item->landscape, classId});
			return 0;
		}
		else
		{
			int closed = 0;
			float closedDist = std::numeric_limits<float>::max();
			for (size_t i = 1; i < landscapes.size(); i++)
			{

				float dist = iterDistance<Distance::Iter::SumNorm2>(landscapes[i].first, item->landscape, 0.1f);
				if (dist < closedDist)
				{
					closedDist = dist;
					closed = landscapes[i].second;
				}
			}
			out.push_back(closed);
			return 1;
		}
	}

	void predict()
	{

	}

	int test(int id)
	{
		return out[id];
	}

	void setClasses(int n)
	{
	}

};
