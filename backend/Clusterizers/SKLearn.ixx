module;

#include <memory>
#include <functional>
#include <vector>
#include <cassert>
#include <iostream>


export module Sklearn;

import ClusterInterface;
import TreeSignClass;
import ExteranlReader;

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

	bool predict(const IClusterItemHolder& iallItems)
	{
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
			auto& sign = allItems.getItem(i).signature;

			for (const auto& num : sign)
			{
				tempFile << num << " ";
			}
			tempFile.seekp(-1, tempFile.cur); // ������� ��������� �������
			tempFile << std::endl;

		}
		tempFile.close();

		BackString execCmd = "python.exe ";
		execCmd += (Variables::metaPath / "cluster.py").string() + " ";
		execCmd += filePath + " ";
		execCmd += settings.getEnum("method");
		execCmd += " '";
		execCmd += getPythonSettings(settings);
		execCmd += "'";
		return exec(execCmd, cachedAssignments, n);

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
