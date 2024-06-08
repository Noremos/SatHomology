#ifdef USE_MODULE
module;
#else
#pragma once
#endif

#include <vector>
#include <cassert>
#include <iostream>
#include "Common.h"


#include "../Interfaces/ICluster.h"
#include "ExteranlReader.h"
#include "TreeSignClass.h"

#ifdef USE_MODULE
export module Sklearn;
#define MEXPORT export
#else
#define MEXPORT
#endif
//import BackBind;
// import ClusterInterface;
// import TreeSignClass;
// import ExteranlReader;

MEXPORT class ISklearnClassifier : public IBarClusterizer
{
	int n;
	std::vector<unsigned long> cachedAssignments;
public:
	ISklearnClassifier()
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
		// n = size;
	}

	int getClusters()
	{
		return n;
	}

	virtual void writeToTemp(const IClusterItemHolder& iallItems, BackFileWriter &tempFile) = 0;
	bool predict(const IClusterItemHolder& iallItems)
	{
		n = *IBarClusterizer::settings.getInt("n_clusters");

		BackString filePath = get_temp_file_path();
		BackFileWriter tempFile(filePath, BackFileWriter::out | BackFileWriter::trunc);
		if (!tempFile.is_open())
		{
			std::cerr << "Unable to open temporary file for writing." << std::endl;
			exit(EXIT_FAILURE);
		}

		writeToTemp(iallItems, tempFile);

		tempFile.close();


		std::vector<BackString> execCmd;
		execCmd.push_back(getPythonExe());
		execCmd.push_back((Variables::metaPath / "cluster.py").string());
		execCmd.push_back(filePath);
		execCmd.push_back(BackString(settings.getEnum("method").data()));
		execCmd.push_back(getPythonSettings(settings));
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
