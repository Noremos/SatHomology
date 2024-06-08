#include <memory>
#include <functional>
#include <vector>
#include <cassert>
#include <iostream>
#include "Common.h"

#include "../Interfaces/ICluster.h"
#include "ExteranlReader.h"
#include "TreeSignClass.h"



// import ClusterInterface;
// import TreeSignClass;
// import ExteranlReader;
//import BackBind;

class TSlearnClassifier : public IBarClusterizer
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
		// n = size;
	}

	int getClusters()
	{
		return n;
	}

	bool predict(const IClusterItemHolder& iallItems)
	{
		n = *IBarClusterizer::settings.getInt("n_clusters");

		const TreeSignatureCollection& allItems = dynamic_cast<const TreeSignatureCollection&>(iallItems);
		BackString filePath = get_temp_file_path();
		BackFileWriter tempFile(filePath, BackFileWriter::out | BackFileWriter::trunc);
		if (!tempFile.is_open())
		{
			std::cerr << "Unable to open temporary file for writing." << std::endl;
			exit(EXIT_FAILURE);
		}

		for (size_t i = 0; i < allItems.getItemsCount(); i++)
		{
			auto& sign = allItems.getRItem(i).signature;

			for (const auto& num : sign)
			{
				tempFile << num << " ";
			}
			tempFile.seekp(-1, tempFile.cur); // ������� ��������� �������
			tempFile << std::endl;

		}
		tempFile.close();

		std::vector<BackString> execCmd;
		execCmd.push_back(getPythonExe());
		execCmd.push_back((Variables::metaPath / "tslearn.py").string());
		execCmd.push_back(filePath);
		execCmd.push_back(BackString(settings.getEnum("method").data()));
		execCmd.push_back(getPythonSettings(settings));
		return exec(execCmd, cachedAssignments, n);
	}

	int test(size_t itemId)
	{
		return cachedAssignments[itemId];
	}
};


GlobalClusterRegister<TreeClass, TreeSignatureCollection, TSlearnClassifier> TSLearnReg("TSLearn");
