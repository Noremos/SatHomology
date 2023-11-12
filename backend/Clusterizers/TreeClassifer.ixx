module;
#include <memory>
#include <functional>
#include <vector>
#include <assert.h>

#include "../../Bind/Common.h"
#include "../../side/kde.h"

#include <dlib/clustering.h>

export module TreeClassifier;

import BarcodeModule;
import ClusterInterface;
import IItemModule;

import IOCore;
import Platform;

import TrainIO;
import MHashMap;



class TreeClass : public IClassItem
{
public:
	// int id;
	std::vector<short> signature;

	// IClassItem* parent;
	TreeClass(bc::barline* line = nullptr)
	{
		if (line)
		{
			walk(line, 0);
		}
	}

	virtual size_t getId() const
	{
		return 0;
	}

	virtual size_t getParentId() const
	{
		return 0;
	}

	virtual int getDeath() const override
	{
		return 0;
	}

	virtual Barscalar start() const override
	{
		return 0;
	}

	virtual Barscalar end() const override
	{
		return 0;
	}

	virtual const bc::barvector& getMatrix() const
	{
		return {};
	}

	virtual const size_t getMatrixSize() const
	{
		return 0;
	}

	virtual void saveLoadState(StateBinFile::BinState* state) override
	{
		state->beginItem();
		size_t arraySize = 0;
		state->beginArray(signature, arraySize);
		for (size_t i = 0; i < arraySize; i++)
		{
			signature[i] = state->pShort(signature[i]);
		}
	}

private:
	void walk(bc::barline* line, int depth)
	{
		if (line->getChildrenCount() == 0)
		{
			signature.push_back(depth);
		}
		else
		{
			int half = line->getChildrenCount() / 2;
			for (size_t i = 0, total = line->getChildrenCount(); i < total; i++)
			{
				if (i == half)
					signature.push_back(depth);

				walk(line->getChild(i), depth + 1);
			}
		}
	}
};
//

class TreeSignatureCollection: public IDataClassItemHolder<TreeClass>
{
	using Base = IDataClassItemHolder<TreeClass>;
protected:

public:

	void create(bc::DatagridProvider* img, const bc::BarConstructor& constr, const Base::ItemCallback& callback)
	{
		bc::BarcodeCreator creator;
		std::unique_ptr<bc::Barcontainer> ret(creator.createBarcode(img, constr));
		auto* item = ret->getItem(0);
		int size = (int)item->barlines.size();
		for (int i = 0; i < size; i++)
		{
			TreeClass* id = new TreeClass(item->barlines[i]);
			callback(id);
			Base::items.push_back(id);
		}
	}

	virtual void saveLoadState(StateBinFile::BinState* state) override
	{
		// Begin
		size_t linesCount;
		state->beginArray(Base::items, linesCount);
		for (size_t i = 0; i < linesCount; ++i)
		{
			Base::items[i]->saveLoadState(state);
		}
	}

	void add(TreeClass* sign)
	{
		Base::items.push_back(sign);
	}

	IClassItem* getItem(size_t id)
	{
		return items[id];
	}

	IClassItem* exractItem(size_t id)
	{
		auto* temp = items[id];
		items[id] = nullptr;
		return temp;
	}

	const IClassItem* getItem(size_t id) const
	{
		return items[id];
	}

	size_t count()
	{
		return items.size();
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

	void predict(const IClassItemHolder& allItems)
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
			kdepp::Kde1d<short> kernel(static_cast<const TreeClass*>(allItems.getItem(i))->signature);
			auto result = kernel.eval(0.5);
			m(0) = result;
			samples.push_back(m);
		}

		test.set_number_of_centers(n);
		pick_initial_centers(n, initial_centers, samples, test.get_kernel());
		test.train(samples, initial_centers);
		cachedAssignments = spectral_cluster(kernel_type(0.1), samples, n);
	}

	virtual int test(size_t itemId)
	{
		return cachedAssignments[itemId];
	}

};


GlobalClusterRegister<TreeClass, TreeSignatureCollection, TreeClassifier> c("Сигнатура по дереву");
