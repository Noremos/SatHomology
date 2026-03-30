#include <vector>
#include <memory>

#include <random>
#include <limits>

#include <filesystem>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include <limits>
// #include <map>

#include "Barcode/PrjBarlib/include/barline.h"
#include "Barcode/PrjBarlib/include/barcodeCreator.h"

#include "../MatrImg.h"
#include "../MLSettings.h"
#include "../Layers/layerInterface.h"
#include "../Layers/Rasterlayers.h"

#include "../backend/project.h"
#include "../backend/Interfaces/IAlgorithm.h"
//import AlgUtils;
//import SimpleBar;
#include "../backend/Algs/SimpleBar.h"
#include "../Algs/Gen2/Trainer.h"


#include "Hungarian.h" // Подключаем библиотеку для Венгерского алгоритма (lap.h)
#include "../Clusterizers/ExteranlReader.h"

class MatrixXd
{
public:
	int rows, cols;
	std::vector<double> data;
	MatrixXd(int rows, int cols) : rows(rows), cols(cols)
	{
		data.resize(rows * cols);
	}

	double& operator()(int i, int j)
	{
		return get(i, j);
	}
	double operator()(int i, int j) const
	{
		assert(i >= 0 && i < rows && j >= 0 && j < cols);
		auto value = data[i * cols + j];
		assert(value == data[j * cols + i]);
		return value;
	}

	double& get(int i, int j)
	{
		assert(i >= 0 && i < rows && j >= 0 && j < cols);
		double& value = data[i * cols + j];
		assert(value == data[j * cols + i]);
		return value;
	}


	void setBoth(int i, int j, double val)
	{
		assert(i >= 0 && i < rows && j >= 0 && j < cols);
		assert(data[i * cols + j] == data[j * cols + i]);
		data[j * cols + i]  = data[i * cols + j] = val;
	}

	std::vector<std::vector<double>> getMatr() const
	{
		std::vector<std::vector<double>> m;
		m.resize(rows);
		for (size_t i = 0; i < rows; i++)
		{
			m[i].resize(rows);

			for (size_t j = 0; j < rows; j++)
			{
				m[i][j] = operator()(i, j);
			}
		}

		return m;
	}

	MatrixXd(const MatrixXd& other) : rows(other.rows), cols(other.cols)
	{
		data = other.data;
	}

	MatrixXd& operator=(const MatrixXd& other)
	{
		rows = other.rows;
		cols = other.cols;
		data = other.data;
		return *this;
	}

	MatrixXd& operator=(MatrixXd&& other)
	{
		if (this == &other)
			return *this;

		rows = other.rows;
		cols = other.cols;
		data = std::move(other.data);
		return *this;
	}


	void updateDistances(int source, int cooked)
	{
		double maxVal = get(source, cooked);
		for (size_t i = 0; i < rows; i++)
		{
			setBoth(i, cooked,  std::max(get(i, cooked), maxVal));
		}

	}


	std::pair<int, int> findMinDistance(int numClusters)
	{
		double minDistance = std::numeric_limits<double>::max();
		std::pair<int, int> minPair = { -1, -1 };
		assert(rows > 0);
		assert(data.size() > 0);
		using namespace std;
		for (int i = 0; i < rows; ++i)
		{
			for (int j = i + 1; j < cols; ++j)
			{
				double val = get(i, j);
				if (val < 0.0) // skip -1.0
					continue;

				// cout << val <<  " vs " << minDistance << endl;
				if (val < minDistance)
				{
					minDistance = val;
					minPair = { i, j };
				}
			}
		}
		return minPair;
	}

	void fill(int value)
	{
		std::fill(data.begin(), data.end(), value);
	}
};



struct ClassicTrainer
{
	std::vector<std::unique_ptr<bc::Baritem>> constrs;
	std::vector<uint32_t> labels;


	void add(bc::Baritem* item, const uint16_t typeId)
	{
		item->sortByLen();

		constrs.emplace_back(item);
		labels.emplace_back(typeId);
	}

	std::vector<unsigned long> output;

	void predict()
	{
		int n = constrs.size();

		MatrixXd matrix(n, n);

		double maxval = 0;

		for (size_t i = 0; i < n; i++)
		{
			for (size_t j = i + 1; j < n; j++)
			{
				matrix.setBoth(i, j,  constrs[i]->compareFull(constrs[j].get(), bc::CompareStrategy::CommonToLen, false));
				if (matrix.get(i, j) > maxval)
					maxval = matrix.get(i, j);
			}
			matrix(i, i) = 0;
		}

		BackString filePath = get_temp_file_path();
		StateBinFile::BinStateWriter writer;
		filePath.replace(filePath.size() - 3, 3, "bin");
		writer.open(filePath);
		writer.pInt(n);
		for (size_t i = 0; i < n; i++)
		{
			for (size_t j = 0; j < n; j++)
			{
				writer.pFloat(matrix.get(i, j));
			}
		}
		writer.close();


		std::vector<BackString> execCmd;
		execCmd.push_back(getPythonExe());
		execCmd.push_back((Variables::metaPath / "cluster_distance_matrix.py").string());
		execCmd.push_back(filePath);
		execCmd.push_back("mds_kmeans");
		execCmd.push_back(to_string(3));

		int dummyN = n;
		exec(execCmd, output, dummyN, false);
	}

	uint16_t test(int32_t id)
	{
		return labels[id];
	}
};


class DatasetWork1
{
	std::unordered_map<BackPathStr, int> sourceFiles;
	int NC = 2;
public:
	ClassicTrainer ct;

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

	void collect(int maxAllowed, const bc::barstruct& constr)
	{
		std::cout << "Sample " << maxAllowed << " elements from each cluster" << std::endl;

		added = 0;
		totalAdded.clear();
		totalAdded.resize(NC, 0);


		correctIds = {};
		for (auto& entry : sourceFiles)
		{
			int correctId = entry.second;

			BackPathStr path =  entry.first;
			// assert(pathExists(path));

			totalAdded[correctId]++;
			BackImage main = imread(path);

			ct.add(bc::BarcodeCreator::create(main, constr).release(), correctId);

			added += 1;
			if (added % 20 == 0)
				cout << added << "/" << sourceFiles.size() << endl;

			correctIds.push_back(correctId);
		}

	}

	std::pair<int, int> predict()
	{
		std::cout << "Predicting..." << std::endl;
		ct.predict();
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
			predictions.push_back(ct.test(i));
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

		std::vector<int> new_predicted_labels(predicted_labels.size());
		for (size_t i = 0; i < predicted_labels.size(); ++i) {
			new_predicted_labels[i] = label_mapping[predicted_labels[i]];
		}
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


void experiment4classes(const bc::barstruct& constr)
{
	int maxAllowed = 100;

	DatasetWork1 dws;
	dws.openCraters(maxAllowed, "objects/NWPU-RESISC45", {"desert", "intersection", "railway", "commercial_area"});
	dws.collect(maxAllowed, constr);

	BackFileWriter result("result.txt", std::ios::trunc);

	auto res = dws.predict();
	result << " => " << res.first << "/" << res.second << ", " << ((100.f * res.first) / res.second) << "%" << endl;

	cout << "DONE" << endl;
}

// AutoRunRegister registerParseCocoAnnotations([]()
// {
// 	bc::barstruct constr;
// 	constr.proctype = bc::ProcType::Radius;
// 	constr.createGraph = true;
// 	constr.createBinaryMasks = false;
// 	constr.attachMode = bc::AttachMode::morePointsEatLow;
// 	experiment4classes(constr);

// 	exit(0);
// });