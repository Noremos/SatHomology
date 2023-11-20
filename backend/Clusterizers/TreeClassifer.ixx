module;
#include <memory>
#include <functional>
#include <vector>
#include <cassert>

#include "../../Bind/Common.h"
#include "../../side/kde.h"

#include <dlib/clustering.h>

export module TreeClassifier;

import ClusterInterface;

import Platform;

import TrainIO;
import MHashMap;
import CachedBarcode;
import MLSettings;



class TreeClass : public ICluster
{
public:
	std::vector<short> signature;
	bc::barvector matrix;
	bool addMiddleDepth;

	// IClassItem* parent;
	//TreeClass(CachedBarline* line = nullptr) //: id(id)
	//{
	//	if (line)
	//	{
	//		walk(line, 0);
	//	}
	//}

	const bc::barvector& getMatrix() const
	{
		return matrix;
	}

	TreeClass()
	{ }

	explicit TreeClass(MLSettings& options, const CachedBarline& line) //: id(id)
	{
		addMiddleDepth = options.getInt("addMiddleDepth");
		walk(&line, 0);
		matrix = line.getMatrix();
	}

	explicit TreeClass(const TreeClass& other) //: id(id)
	{
		signature = other.signature;
		matrix = other.matrix;
	}

	explicit TreeClass(TreeClass&& other) //: id(id)
	{
		signature = std::move(other.signature);
		matrix = std::move(other.matrix);
	}
	//virtual void saveLoadState(StateBinFile::BinState* state) override
	//{
	//	// Throw
	//	state->beginItem();
	//	size_t arraySize = 0;
	//	state->beginArray(signature, arraySize);
	//	for (size_t i = 0; i < arraySize; i++)
	//	{
	//		signature[i] = state->pShort(signature[i]);
	//	}
	//}

private:
	void walk(const CachedBarline* line, int depth)
	{
		if (line->getChildrenCount() == 0)
		{
			signature.push_back(depth);
		}
		else
		{
			if (addMiddleDepth)
				signature.push_back(depth);

			//int half = line->getChildrenCount() / 2;
			for (size_t i = 0, total = line->getChildrenCount(); i < total; i++)
			{
				//if (i == half)
				//	signature.push_back(depth);

				walk(line->getChild(i), depth + 1);
			}
		}
	}
};
//

class TreeSignatureCollection: public IClusterItemValuesHolder<TreeClass>
{
	using Base = IClusterItemValuesHolder<TreeClass>;
protected:

public:
	TreeSignatureCollection()
	{
		Base::settings =
		{
			{"addMiddleDepth", false},
			{"minSignatureSize", 2}
		};
	}

	virtual void addItem(const CachedBarline& item)
	{
		TreeClass clusterItem(Base::settings, item);
		if (clusterItem.signature.size() > *Base::settings.getInt("minSignatureSize"))
		{
			Base::items.push_back(std::move(clusterItem));
		}
	}
};

export class TreeClassifier : public IBarClusterizer
{
	int n;
	std::vector<unsigned long> cachedAssignments;
public:
	const BackString name() const
	{
		return "TREE_SIGNATURE";
	}

	void setClassesCount(int size)
	{
		n = size;
	}


	//void loadData()
	//{
	//	//ClassDataIO io;
	//	//io.open(IBarClusterizer::dbPath);
	//	//clusters = categs;
	//	//ClassDataIO::TrainCallback cla = [this](int clId, vbuffer& buf, BackImage, size_t dbLocalId)
	//	//{
	//	//	std::stringstream stream;
	//	//	stream.write(reinterpret_cast<const char*>(buf.data()), buf.size());
	//	//	stream.seekg(0, std::ios::beg);
	//	//	// std::istringstream stream(buf.data(), buf.size());

	//	//	TItem raw{};
	//	//	raw.read(stream); // Already Prepared
	//	//	addDataInner(clId, &raw, dbLocalId);
	//	//};

	//	//io.loadAll(cla, -1, ClassDataIO::LF_ALL);
	//}

	//void loadClassData(ClassDataIO& io)
	//{

	//}

	//int addClass(const BackString& name)
	//{
	//	return clusters.addValue(name);
	//}

	//void removeClass(int id)
	//{
	//	clusters.remove(id);
	//}

	void predict(const IClusterItemHolder& allItems)
	{
		using namespace dlib;

		//typedef std::vector<short> sample_type;
		typedef matrix<double, 1, 1> sample_type;
		sample_type m;
		// Now we are making a typedef for the kind of kernel we want to use.  I picked the
		// radial basis kernel because it only has one parameter and generally gives good
		// results without much fiddling.
		typedef radial_basis_kernel<sample_type> kernel_type;

		kcentroid<kernel_type> kc(kernel_type(0.1), 0.01, 8);
		kkmeans<kernel_type> test(kc);

		std::vector<sample_type> samples;
		std::vector<sample_type> initial_centers;

		for (size_t i = 0; i < allItems.getItemsCount(); i++)
		{
			auto& sign = static_cast<const TreeClass*>(allItems.getItem(i))->signature;
			//if (sign.size() > 1)
			{
				kdepp::Kde1d<short> kernel(sign);
				auto result = kernel.eval(0.5);
				m(0) = result;
			}
			//else
			//	m(0) = sign.front();

			samples.push_back(m);
		}

		test.set_number_of_centers(n);
		pick_initial_centers(n, initial_centers, samples, test.get_kernel());
		test.train(samples, initial_centers);
		cachedAssignments = spectral_cluster(kernel_type(0.1), samples, n);
	}

	int test(size_t itemId)
	{
		return cachedAssignments[itemId];
	}
};


GlobalClusterRegister<TreeClass, TreeSignatureCollection, TreeClassifier> c("Сигнатура по дереву");


export class TreeSizeClassifier : public IBarClusterizer
{
	int n;
	std::vector<unsigned long> cachedAssignments;
public:
	const BackString name() const
	{
		return "TREE_SIZE_SIGNATURE";
	}

	void setClassesCount(int size)
	{
		n = size;
	}

	void predict(const IClusterItemHolder& allItems)
	{
		using namespace dlib;

		typedef matrix<double, 1, 1> sample_type;
		sample_type m;

		typedef radial_basis_kernel<sample_type> kernel_type;

		kcentroid<kernel_type> kc(kernel_type(0.1), 0.01, 8);
		kkmeans<kernel_type> test(kc);

		std::vector<sample_type> samples;
		std::vector<sample_type> initial_centers;
		for (size_t i = 0; i < allItems.getItemsCount(); i++)
		{
			auto& sign = static_cast<const TreeClass*>(allItems.getItem(i))->signature;
			m(0) = sign.size();

			samples.push_back(m);
		}

		test.set_number_of_centers(n);
		pick_initial_centers(n, initial_centers, samples, test.get_kernel());
		test.train(samples, initial_centers);
		cachedAssignments = spectral_cluster(kernel_type(0.1), samples, n);
	}

	int test(size_t itemId)
	{
		return cachedAssignments[itemId];
	}
};


GlobalClusterRegister<TreeClass, TreeSignatureCollection, TreeSizeClassifier> c0("Длина сигнатуры");
