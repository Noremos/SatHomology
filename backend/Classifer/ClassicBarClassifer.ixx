module;
#include <memory>
#include <random>
#include <iostream>
#include <functional>
#include <vector>
#include <assert.h>

#include "../../Bind/Common.h"

export module Classifiers;

import BarcodeModule;
import ClassifierInterface;

import IOCore;
import Platform;

import BarcodeModule;
import TrainIO;
import MHashMap;


export class BarlineClass : public IClassItem
{
public:
	// int id;
	bc::barline* line = nullptr;
	int depth = 0;
	int matrSize = 0;
	bc::barlinevector childer;

	// IClassItem* parent;
	BarlineClass(bc::barline* line = nullptr) : line(line)
	{
		if (line)
		{
			depth = line->getDeath();
			matrSize = line->matr.size();
			update();
		}
	}

	virtual size_t getId() const
	{
		return (size_t)line;
	}

	virtual size_t getParentId() const
	{
		return (size_t)line->parent;
	}

	virtual int getDeath() const override
	{
		return depth;
	}

	virtual Barscalar start() const override
	{
		return line->start;
	}

	virtual Barscalar end() const override
	{
		return line->end();
	}

	virtual const bc::barvector& getMatrix() const
	{
		return line->matr;
	}

	virtual const size_t getMatrixSize() const
	{
		return matrSize;
	}

	void update()
	{
		line->getChilredAsList(childer, true, true, false);
	}

	virtual bool passFilter(const FilterInfo& filter) const
	{
		return filter.start.inRange(line->start) &&
			filter.len.inRange(line->len()) &&
			filter.matrSizeProc.inRange(line->matr.size() * 100 / filter.imgLen) &&
			filter.depth.inRange(line->getDeath());
	}

	virtual void saveLoadState(StateBinFile::BinState* state) override
	{
		if (state->isReading())
			line = new bc::barline();

		state->beginItem();
		state->pType(line->start.type); // Only set

		line->start = state->pBarscalar(line->start);
		line->m_end = state->pBarscalar(line->end());
		matrSize = state->pInt(matrSize);
		depth = state->pInt(depth);

		int csize = childer.size();
		csize = state->pInt(csize);
		state->beginArray(childer, csize);
		for (size_t i = 0; i < csize; i++)
		{
			if (state->isReading())
				childer[i] = new bc::barline();

			auto cline = childer[i];

			cline->start = state->pBarscalar(cline->start);
			cline->m_end = state->pBarscalar(cline->end());
		}
	}
};


export class BaritemHolder : public IDataClassItemHolder<IClassItem>
{
	std::shared_ptr<bc::Baritem> item;

public:
	~BaritemHolder()
	{
		for (size_t var = 0; var < items.size(); ++var)
		{
			delete items[var];
		}
		items.clear();
	}

	void create(bc::DatagridProvider* img, const bc::BarConstructor& constr, const ItemCallback& callback)
	{
		bc::BarcodeCreator creator;
		std::unique_ptr<bc::Barcontainer> ret(creator.createBarcode(img, constr));

		item.reset(ret->exractItem(0));
		int size = item->barlines.size();
		for (int i = 0; i < size; i++)
		{
			BarlineClass id(item->barlines[i]);
			callback(&id);
		}
	}

	void create(bc::DatagridProvider* img, const bc::BarConstructor& constr)
	{
		bc::BarcodeCreator creator;
		std::unique_ptr<bc::Barcontainer> ret(creator.createBarcode(img, constr));

		item.reset(ret->exractItem(0));

		int size = item->barlines.size();
		for (int i = 0; i < size; i++)
		{
			IClassItem* id = new BarlineClass(item->barlines[i]);
			items.push_back(id);
		}
	}

	virtual void saveLoadState(StateBinFile::BinState* state) override
	{
		const bool isReading = state->isReading();

		if (isReading)
			item.reset(new bc::Baritem());

		bc::barlinevector& vec = item->barlines;

		// index = state->pInt(index); // Index
		item->setType((BarType)state->pType(item->getType())); // BarType
		size_t linesCount = state->pArray(vec.size());

		// Parent read/write stuff
		uint counterId = 0;
		std::vector<bc::barline*> ids;
		ids.resize(linesCount * 2);
		std::fill(ids.begin(), ids.end(), nullptr);
		MMMAP<size_t, uint> writeIds;

		// Begin
		state->beginArray(vec, linesCount);
		for (size_t i = 0; i < linesCount; ++i)
		{
			bc::barline* line;
			if (!isReading)
			{
				// Write
				line = vec[i];

				uint couId;
				auto p = writeIds.find((size_t)line);
				if (p != writeIds.end())
				{
					couId = p->second;
				}
				else
				{
					couId = counterId++;
					writeIds.insert({ (size_t)line, couId });
				}

				state->pInt(couId);

				uint parId;
				p = writeIds.find((size_t)line->parent);
				if (p != writeIds.end())
				{
					parId = p->second;
				}
				else
				{
					parId = counterId++;
					writeIds.insert({ (size_t)line->parent, parId });
				}

				state->pInt(parId);
			}
			else
			{
				// Read

				// Main line
				uint couId = state->pInt(0);
				line = ids[couId];
				if (line == nullptr)
				{
					line = new bc::barline();
					ids[couId] = line;
				}
				vec[i] = line;

				// Parent
				uint parId = state->pInt(0);
				bc::barline* par = ids[parId];
				if (par == nullptr)
				{
					par = new bc::barline();
					ids[parId] = par;
				}

				line->setparent(par);
			}

			line->start = state->pBarscalar(line->start);
			line->m_end = state->pBarscalar(line->end());

			//act matrSize = state->pFixedArray(line->matr, 4 + typeSize);
			uint matrSize = state->pArray(line->matr.size());
			state->beginArray(line->matr, matrSize);
			for (uint j = 0; j < matrSize; ++j)
			{
				bc::barvalue& v = line->matr[j];
				v.index = state->pInt(v.index);
				v.value = state->pBarscalar(v.value);
			}

			IClassItem* id = new BarlineClass(line);
			items.push_back(id);
		}
		state->endItem();
	}
};


export class barclassificator : public IDataBarClassifier<BarlineClass>
{
public:
	struct ClassData
	{
		int classId;
		bc::Barcontainer container;
		MMMAP<size_t, int> cacheIndex;
	};

	std::vector<std::unique_ptr<ClassData>> classes;
	// BackPathStr dbPath;

	const BackString name() const
	{
		return "CLASSIC";
	}

	void loadData(const BarCategories& categs)
	{
		ClassDataIO io;
		io.open(dbPath);

		// dbPath = path;
		classes.clear();
		for (const BarClassCategor& categ : categs.categs)
		{
			addClass(categ.id);
			loadClassData(io, categ.id);
		}
	}

	void loadClassData(ClassDataIO& io, int classId)
	{
		ClassDataIO::TrainCallback cla = [this](int clId, vbuffer& buf, BackImage, size_t dbLocalId)
		{
			std::stringstream stream;
			stream.write(reinterpret_cast<const char*>(buf.data()), buf.size());
			stream.seekg(0, std::ios::beg);
			// std::istringstream stream(buf.data(), buf.size());

			BarlineClass raw;
			raw.read(stream); // Already Prepared
			addDataInner(clId, &raw, dbLocalId);
		};

		io.loadAll(cla, classId, ClassDataIO::LF_ALL);
	}

	ClassData* getClass(int id)
	{
		for (int i = 0; i < classes.size(); i++)
		{
			if (classes[i]->classId == id)
			{
				return classes[i].get();
			}
		}

		throw;
	}

	void addClass(int id)
	{
		auto nd = std::make_unique<ClassData>();
		nd->classId = id;
		classes.push_back(std::move(nd));
	}

	void removeClass(int id)
	{
		for (int i = 0; i < classes.size(); i++)
		{
			if (classes[i]->classId == id)
			{
				classes.erase(classes.begin() + i);
				break;
			}
		}
	}

	void prepareBeforeAddInner(BarlineClass *raw)
	{
		bc::Baritem item;
		item.barlines = std::move(raw->childer);
		item.relen();
		item.setType();
		raw->childer = std::move(item.barlines);
	}

	void addDataInner(int classInd, BarlineClass* raw, size_t dataId, bool extractLine = false)
	{
		bc::Baritem* item = new bc::Baritem();
		item->barlines = std::move(raw->childer);
		item->setType();

		assert(classes.size() != 0);
		auto* classHolder = getClass(classInd);
		classHolder->cacheIndex.insert(std::make_pair(dataId, classHolder->container.count()));
		classHolder->container.addItem(item);
	}

	// void addData(int classInd, bc::barlinevector& cont, const bool move = true)
	// {
	// 	auto classHolder = getClass(classInd);
	// 	bc::Baritem* item = new bc::Baritem();
	// 	if (move)
	// 	{
	// 		item->barlines = std::move(cont);
	// 	}
	// 	else
	// 	{
	// 		// Copy
	// 		for (size_t j = 0; j < cont.size(); j++)
	// 		{
	// 			item->barlines.push_back(cont[j]->clone());
	// 		}
	// 	}

	// 	item->relen();
	// 	classHolder.container.addItem(item);
	// }

	// void addData(int classInd, bc::Baritem* item)
	// {
	// 	auto classHolder = getClass(classInd);
	// 	item->relen();
	// 	classHolder.container.addItem(item);
	// }


	bool removeDataInner(int classId, size_t id)
	{
		ClassData& classHolder = *getClass(classId);
		auto it = classHolder.cacheIndex.find(id);
		if (it != classHolder.cacheIndex.end())
		{
			delete classHolder.container.exractItem(it->second);
			classHolder.cacheIndex.erase(it);
			return true;
		}
		return false;
	}

	int predictInner(const BarlineClass* raw)
	{
		bc::Baritem newOne;
		raw->line->getChilredAsList(newOne.barlines, true, true, false);
		newOne.relen();

		auto cp = bc::CompireStrategy::CommonToLen;
		float res = 0;

		int maxInd = -1;
		float maxP = res;
		for (size_t i = 0; i < classes.size(); i++)
		{
			float ps = classes[i]->container.compireBest(&newOne, cp);
			if (ps > maxP)
			{
				maxP = ps;
				maxInd = i;
			}
		}

		//		bar0.barlines.clear();
		//		delete bar0;

		assert(maxP <= 1.0);
		return maxP > 0.5 ? maxInd : -1;
	}


	~barclassificator()
	{
		//		for (size_t i = 0; i < N * 2; i++)
		//		{
		//			delete classes[i];
		//		}
	}
private:
	// Barscalar asScalar(const BackJson& arr)
	// {
	// 	return Barscalar(arr[0].asDouble(), arr[1].asDouble(), arr[2].asDouble());
	// }
};

//void parseItem(const JsonObject &obj, bc::barlinevector &lines)
//{
//	bc::barline *line = new bc::barline();
//	line->start = asScalar(obj["start"].array());
//	line->m_end = asScalar(obj["end"].array());

//	lines.push_back(line);

//	auto arrcoors = obj["children"].array();
//	for (size_t i = 0; i < arrcoors.size(); i++)
//	{
//		parseItem(arrcoors.at(i).object(), lines);
//	}
//}
//BarcodesHolder toHoldes(const bc::CloudPointsBarcode::CloudPoints& cloud)
//{
//	bc::CloudPointsBarcode clodCrt;
//	std::unique_ptr<bc::Barcontainer> hold(clodCrt.createBarcode(&cloud));

//	BarcodesHolder holder;
//	if (cloud.points.size() == 0)
//		return holder;

//	bc::Baritem* main = hold->getItem(0);
//	for (size_t var = 0; var < main->barlines.size(); ++var)
//	{
//		auto* line = main->barlines[var];
//		BarcodeHolder* barhold = new BarcodeHolder();
//		holder.lines.push_back(barhold);

//		barhold->matrix = std::move(line->matr);
//		line->getChilredAsList(barhold->lines, true, true, false);
//	}

//	return holder;
//}


//public:
	//void addClass(const BackPathStr& binFile, int classInd)
	//{
	//	BackString val;

	//	std::ifstream file(path.string());
	//	BackJson jsonDocument = BackJson::parse(file);

	//	// �� �������� �������� ������ � ������� ������� QJsonObject
	//	if (jsonDocument.is_array())
	//	{
	//		BackJson jsonItems = jsonDocument.array();

	//		for (size_t i = 0; i < jsonItems.size(); i++)
	//		{
	//			JsonObject jsItem = jsonItems.at(i).object();
	//			bc::Baritem *item = new bc::Baritem();
	//			parseItem(jsItem, item->barlines);
	//			item->relen();
	//			classes[classInd].addItem(item);
	//		}
	//	}
	//	else
	//	{
	//		bc::Baritem *item = new bc::Baritem();
	//		parseItem(jsonDocument.object(), item->barlines);
	//		item->relen();
	//		classes[classInd].addItem(item);
//	}
//}


//void addClass(bc::barline *line, int classInd)
//{
//	bc::Baritem *item = new bc::Baritem();
//	line->getChilredAsList(item->barlines, true, true);
//	item->relen();

//	classes[classInd].addItem(item);
//}

void registerClassic()
{
	ClassFactory::RegisterFactory<BarlineClass, BaritemHolder, barclassificator>(1);
}
