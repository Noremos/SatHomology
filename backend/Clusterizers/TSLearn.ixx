module;
#include <memory>
#include <functional>
#include <vector>
#include <cassert>
#include <iostream>

#include "../../Bind/Common.h"



export module TSLearn;

import ClusterInterface;
import TreeSignClass;
import ExteranlReader;

export class TSlearnClassifier : public IBarClusterizer
{
	int n;
	std::vector<unsigned long> cachedAssignments;
public:
	TSlearnClassifier()
	{
		IBarClusterizer::settings =
		{
			{
				"method",
				{
				//"AffinityPropagation",
				"kmeans",
				"tskmeans_dtw",
				"tskmeans_softdtw",
				"silhouette_score",
			}
		},
		{"n_clusters", 3}
		};
	}
	const BackString name() const
	{
		return "TSLEARN";
	}

	void setClassesCount(int size)
	{
		n = size;
	}

	int getClusters()
	{
		return n;
	}

	bool predict(const IClusterItemHolder& iallItems)
	{
		const TreeSignatureCollection& allItems = dynamic_cast<const TreeSignatureCollection&>(iallItems);
		BackString filePath = get_temp_file_path();
		std::ofstream tempFile(filePath, std::ofstream::out | std::ofstream::trunc);
		if (!tempFile.is_open())
		{
			std::cerr << "Unable to open temporary file for writing." << std::endl;
			exit(EXIT_FAILURE);
		}

		for (size_t i = 0; i < allItems.getItemsCount(); i++)
		{
			auto& sign = allItems.getItem(i).signature;

			for (const auto& num : sign)
			{
				tempFile << num << " ";
			}
			tempFile.seekp(-1, tempFile.cur); // Удаляем последнюю запятую
			tempFile << std::endl;

		}
		tempFile.close();

		BackString execCmd = "python.exe ";
		execCmd += (Variables::metaPath / "tslearn.py").string() + " ";
		execCmd += filePath + " ";
		execCmd += settings.getEnum("method");
		execCmd += " '";
		execCmd += getPythonSettings(settings);
		execCmd += "'";
		return exec(execCmd, cachedAssignments, n);
	}

	int test(size_t itemId)
	{
		return cachedAssignments[itemId];
	}
};


GlobalClusterRegister<TreeClass, TreeSignatureCollection, TSlearnClassifier> c("TSLearn");
