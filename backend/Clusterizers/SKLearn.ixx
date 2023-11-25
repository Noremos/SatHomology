module;
#include <memory>
#include <functional>
#include <vector>
#include <cassert>
#include <iostream>

#include "../../Bind/Common.h"


#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

export module SKLearn;

import ClusterInterface;
import TreeSignClass;

static BackString  get_temp_file_path() {
#ifdef _WIN32
	char buffer[MAX_PATH];
	GetTempPathA(MAX_PATH, buffer);
	BackString temp_path(buffer);
	return temp_path + "input_array.txt";
#else
	return "/tmp/input_array.txt";
#endif
}


export class SklearnClassifier : public IBarClusterizer
{
	int n;
	std::vector<unsigned long> cachedAssignments;
public:
	SklearnClassifier()
	{
		IBarClusterizer::settings =
		{
			{
				"method",
				{
				//"AffinityPropagation",
				"OPTICS",
				"KMeans",
				"DBSCAN",
				"AgglomerativeClustering",
				"Birch",
				"MiniBatchKMeans",
				"MeanShift",
				"SpectralClustering",
				"SpectralBiclustering",
				"SpectralCoclustering",
				"FeatureAgglomeration",
				"BisectingKMeans"
			}
		},
		{"n_clusters", 2},
		{"eps", 0.5},
		{"min_samples", 3},
		{"threshold", 0.5}
		};
	}
	const BackString name() const
	{
		return "SKLEARN";
	}

	void setClassesCount(int size)
	{
		n = size;
	}

	int getClusters()
	{
		return n;
	}


	BackString getPythonSettings()
	{
		BackString json = "{";
		for (auto& set : settings.values)
		{
			if (set.name == "method")
				continue;
			const BackStringView quote = "\\\"";
			json += quote;
			json += set.name;
			json += quote;
			json += ":";

			BackStringView label = set.name;
			switch (set.type)
			{
			case OptionValue::sv_bool:

				if (set.data.b)
					json += "true";
				else
					json += "false";
				break;
			case OptionValue::sv_int:
				json += std::to_string(set.data.i);
				break;
			case OptionValue::sv_double:
			{
				auto s = std::to_string(set.data.d);
				replace_if(s.begin(), s.end(), [](char c) {return c == ','; }, '.');
				json += s;

				break;
			}
			case OptionValue::sv_str:
				json += quote;
				json += *set.data.s;
				json += quote;
				break;
			case OptionValue::sv_enum:
				json += quote;
				json += set.data.e->getSelected();
				json += quote;
				break;
			case OptionValue::sv_path:
				json += quote;
				json += set.data.p->string();
				json += quote;
				break;
			}

			json += ",";
		}
		json.back() = '}';
		//settings.
		return json;
	}


	bool exec(BackStringView cmd)
	{
		std::string outputString;
		std::shared_ptr<FILE> pipe(_popen(cmd.data(), "rt"), _pclose);
		if (!pipe) throw std::runtime_error("_popen() failed!");

		char buffer[128];
		while (!feof(pipe.get()))
		{
			if (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr)
			{
				outputString += buffer;
			}
		}

		printf(outputString.c_str());
		if (outputString.length() == 0)
			return false;

		if (outputString.length() > 1 && outputString[0] == 'E' && outputString[1] == 'R')
		{
			return false;
		}



		outputString.resize(outputString.length() - 1); // Skip last '\n'
		std::ifstream iss;
		iss.open(outputString);
		iss >> n;
		printf("%d\n", n);
		int number;
		while (iss >> number)
			cachedAssignments.push_back(number);

		return true;
	}

	bool predict(const IClusterItemHolder& allItems)
	{
		BackString filePath = get_temp_file_path();
		std::ofstream tempFile(filePath, std::ofstream::out | std::ofstream::trunc);
		if (!tempFile.is_open())
		{
			std::cerr << "Unable to open temporary file for writing." << std::endl;
			exit(EXIT_FAILURE);
		}

		for (size_t i = 0; i < allItems.getItemsCount(); i++)
		{
			auto& sign = static_cast<const TreeClass*>(allItems.getItem(i))->signature;

			for (const auto& num : sign)
			{
				tempFile << num << " ";
			}
			tempFile.seekp(-1, tempFile.cur); // Удаляем последнюю запятую
			tempFile << std::endl;

		}
		tempFile.close();

		BackString execCmd = "python.exe ";
		execCmd += (Variables::metaPath / "cluster.py").string() + " ";
		execCmd += filePath + " ";
		execCmd += settings.getEnum("method");
		execCmd += " '";
		execCmd += getPythonSettings();
		execCmd += "'";
		return exec(execCmd);

		//test.set_number_of_centers(n);
		//pick_initial_centers(n, initial_centers, samples, test.get_kernel());
		//test.train(samples, initial_centers);
		//cachedAssignments = spectral_cluster(kernel_type(0.1), samples, n);
	}

	int test(size_t itemId)
	{
		return cachedAssignments[itemId];
	}
};


GlobalClusterRegister<TreeClass, TreeSignatureCollection, SklearnClassifier> c("SKLearn");
