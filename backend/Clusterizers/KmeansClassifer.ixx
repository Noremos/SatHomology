module;
#include <memory>
#include <functional>
#include <vector>
#include <cassert>

#include "../../Bind/Common.h"
#include <dlib/clustering.h>

export module TreeClassifier;

import ClusterInterface;

import Platform;

import TrainIO;
import MHashMap;
import CachedBarcode;
import MLSettings;
import TreeSignClass;


export class KmeansClassifier : public IBarClusterizer
{
	int n;
	std::vector<unsigned long> cachedAssignments;
public:
	KmeansClassifier()
	{
		IBarClusterizer::settings =
		{
			{"n_clusters", 2}
		};
	}
	const BackString name() const
	{
		return "TREE_SIGNATURE";
	}

	void setClassesCount(int size)
	{
		n = size;
	}

	int getClusters()
	{

		return *IBarClusterizer::settings.getInt("n_clusters");
	}
	bool predict(const IClusterItemHolder& allItems)
	{
		using namespace dlib;

		typedef matrix<double, 1, 1> sample_type;
		sample_type m;

		typedef radial_basis_kernel<sample_type> kernel_type;

		kcentroid<kernel_type> kc(kernel_type(0.1), 0.01, 8);
		kkmeans<kernel_type> test(kc);

		std::vector<sample_type> samples;
		std::vector<sample_type> initial_centers;
		n = getClusters();
		for (size_t i = 0; i < allItems.getItemsCount(); i++)
		{
			auto& sign = static_cast<const TreeClass*>(allItems.getItem(i))->signature;
			m(0) = sign[0];

			samples.push_back(m);
		}

		test.set_number_of_centers(n);
		pick_initial_centers(n, initial_centers, samples, test.get_kernel());
		test.train(samples, initial_centers);
		cachedAssignments = spectral_cluster(kernel_type(0.1), samples, n);
		return true;
	}

	int test(size_t itemId)
	{
		return cachedAssignments[itemId];
	}
};


GlobalClusterRegister<TreeClass, TreeSignatureCollection, KmeansClassifier> c0("kmean -cpp");

