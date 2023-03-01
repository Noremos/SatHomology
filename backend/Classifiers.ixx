module;
#include <memory>
#include <random>
#include <iostream>
#include <unordered_set>
#include <functional>

#include "../Bind/Common.h"

export module ClassifiersCore;
//import std.core;


import BarholdersModule;
import IOCore;
import Platform;
import CacheFilesModule;

import LayersCore;
import BarcodeModule;
import GeoprocessorModule;


export struct BarCategories
{
	int counter;
	std::vector<int> value;
	std::vector<std::string> name;

	int addValue(const BackString& nname)
	{
		if (value.size() > 0 && value.back() > counter)
			counter = value.back() + 1;
		name.push_back(nname);
		int id = counter++;
		value.push_back(id);
		return id;
	}
};


export class barclassificator
{
public:
	BarCategories categs;
	std::vector <bc::Barcontainer> classes;

	void addData(int classInd, bc::barlinevector& cont, const bool move = true)
	{
		bc::Baritem* item = new bc::Baritem();
		if (move)
		{
			item->barlines = std::move(cont);
		}
		else
		{
			// Copy
			for (size_t j = 0; j < cont.size(); j++)
			{
				item->barlines.push_back(cont[j]->clone());
			}
		}

		item->relen();
		classes[classInd].addItem(item);
	}

	void addData(int classInd, bc::Baritem* item)
	{
		item->relen();
		classes[classInd].addItem(item);
	}


	void udpdateClasses()
	{
		for (size_t i = classes.size(); i < categs.value.size(); i++)
		{
			classes.push_back(bc::Barcontainer());
		}
	}

	void removeLast(int classInd)
	{
		classes[classInd].remoeLast();
	}

	////	int getType(bc::barlinevector &bar0lines)
	//int getType(const BarcodeHolder* bar0)
	//{
	//	auto cp = bc::CompireStrategy::CommonToLen;
	//	float res = 0;

	//	//		bc::Baritem *bar0 = new bc::Baritem();
	//	//		bar0->barlines = bar0lines;
	//	bc::Baritem newOne;
	//	bar0->cloneLines(newOne.barlines);
	//	//		newOne.shdowCopy = true;
	//	newOne.relen();

	//	int maxInd = -1;
	//	float maxP = res;
	//	for (size_t i = 0; i < classes.size(); i++)
	//	{
	//		float ps = classes[i].compireBest(&newOne, cp);
	//		if (ps > maxP)
	//		{
	//			maxP = ps;
	//			maxInd = i;
	//		}
	//	}

	//	//		bar0.barlines.clear();
	//	//		delete bar0;

	//	assert(maxP <= 1.0);
	//	return maxP > 0.5 ? maxInd : -1;
	//}


	~barclassificator()
	{
		//		for (size_t i = 0; i < N * 2; i++)
		//		{
		//			delete classes[i];
		//		}
	}
private:
	Barscalar asScalar(const BackJson& arr)
	{
		return Barscalar(arr.at(0).get<double>(), arr.at(1).get<double>(), arr.at(2).get<double>());
	}
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



export struct FilterInfo
{
	struct FRange
	{
		int first;
		int second;

		//template<class T>
		//bool inRange(const T& val) const
		//{
		//	return val >= first && val <= second;
		//}

		bool inRange(int val) const
		{
			return val >= first && val <= second;
		}

		bool inRange(const Barscalar& val) const
		{
			return val >= first && val <= second;
		}

		bool notInRange(int val) const
		{
			return val < first || val > second;
		}

		bool notInRange(const Barscalar& val) const
		{
			return val < first && val > second;
		}
	};

	FRange start{ 0,255 };
	FRange len{ 0,255 };
	FRange matrSizeProc{ 0,100 };
	FRange depth{ 0,1000 };
	int imgLen = 0;

	bool needSkip(bc::barline* line) const
	{
		return start.notInRange(line->start) ||
			len.notInRange(line->len()) ||
			matrSizeProc.notInRange(line->matr.size() * 100 / imgLen) ||
			depth.notInRange(line->getDeath());
	}
};

export class BarClasser
{
protected:
	//barclassificator classer;

	const FilterInfo* filter = nullptr;
	bool cache = true;
public:

	BarClasser(const FilterInfo* filter = nullptr) :
		filter(filter)
	{ 
		if (colors.size() == 0)
		{
			srand(300);
			BarClasser::colors.push_back(Barscalar(255, 0, 0));
			BarClasser::colors.push_back(Barscalar(0, 0, 0));
			BarClasser::colors.push_back(Barscalar(0, 255, 0));
			BarClasser::colors.push_back(Barscalar(0, 0, 255));
			for (int k = 0; k < 40; ++k)
				BarClasser::colors.push_back(Barscalar(5 + rand() % 250, 5 + rand() % 250, 5 + rand() % 250));

			BarClasser::colors.push_back(Barscalar(255, 255, 255));
		}
	}

	void setFilter(FilterInfo* _filter)
	{
		this->filter = _filter;
	}

	void doCache(bool doIt)
	{
		cache = doIt;
	}

	static std::vector<Barscalar> colors;

	virtual bool canRead() const = 0;
	virtual void openRead(const BackPathStr& path) = 0;

	virtual void classBarcodeFromCache(RasterLineLayer& layer, const LayerProvider& prov) = 0;

	//
	virtual void prepare(const BackPathStr& cacheFilePath) = 0;
	virtual void create(int tileIndex, bc::DatagridProvider* img, const bc::BarConstructor& constr, const LayerProvider& prov, RasterLineLayer* layer = nullptr) = 0;
};


std::vector<Barscalar> BarClasser::colors;

//template <typename T, typename TItem, typename THolder>
//concept InheritsFromGeoBar = std::is_base_of<GeoBar<TItem, THolder>, T>::value;


export template<class TCACHER, class TITEM>
class BarClasserInnter : public BarClasser
{
protected:
	TCACHER cacher;

public:
	BarClasserInnter(const FilterInfo* filter = nullptr) :
		BarClasser(filter)
	{ }

	void prepare(const BackPathStr& cacheFilePath)
	{
		cacher.openWrite(cacheFilePath);
	}

	bool canRead() const
	{
		return cacher.canRead();
	}

	void openRead(const BackPathStr& path)
	{
		cacher.openRead(path);
	}


	void classBarcodeFromCache(RasterLineLayer& layer, const LayerProvider& prov)
	{
		int index = -1;
		std::unique_ptr<TITEM> baritem(cacher.load(index));
		std::cout << "Readed from cache index " << index << std::endl;

		classBarcodeInner(*baritem.get(), index, layer, prov);
	}

	void classBarcode(const TITEM* item, int tileIndex, RasterLineLayer& layer, const LayerProvider& prov)
	{
		classBarcodeInner(*item, tileIndex, layer, prov);
	}

private:
	virtual void classBarcodeInner(const TITEM& baritem, int tileIndex, RasterLineLayer& layer, const LayerProvider& prov) = 0;
};

export class RasterBarClasser : public BarClasserInnter<GeoBarRasterCache, bc::Baritem>
{
	bc::BarcodeCreator creator;
	//GeoBarRasterCache cacher;
	//using TITEM = bc::Baritem;
//
public:

	RasterBarClasser(const FilterInfo* filter = nullptr) : BarClasserInnter(filter)
	{ }


	void create(int tileIndex, bc::DatagridProvider* img, const bc::BarConstructor& constr, const LayerProvider& prov, RasterLineLayer* layer = nullptr)
	{
		std::unique_ptr<bc::Barcontainer> ret(creator.createBarcode(img, constr));

		//if (filter)
		//{

		//}

		if (cache)
			cacher.save(ret->getItem(0), tileIndex);

		if (layer)
		{
			classBarcodeInner(*ret->getItem(0), tileIndex, *layer, prov);
		}
	}

	// ------------ Read

private:
	void classBarcodeInner(const bc::Baritem& baritem, int tileIndex, RasterLineLayer& layer, const LayerProvider& prov)
	{
		bool showBad = true;
		bool showWhite = true; //extra.indexOf("showw;") != -1;

		auto tileProv = prov.tileByIndex(tileIndex);
		MMMAP<size_t, std::shared_ptr<SimpleLine>> parentne;
		auto& vec = baritem.barlines;
		for (size_t i = 0; i < vec.size(); ++i)
		{
			auto* curLine = vec.at(i);
			if (filter->needSkip(curLine))
				continue;

			const auto& matr = curLine->matr;

			if (matr.size() == 0)
				continue;

			Barscalar pointCol(255, 0, 0);
			pointCol = colors[rand() % colors.size()];

			std::unordered_set<uint> vals;
			std::shared_ptr<SimpleLine> sl;
			auto curIdKey = (size_t)curLine;
			auto p = parentne.find(curIdKey);
			if (p != parentne.end())
			{
				sl = p->second;
				sl->barlineIndex = (int)i;
			}
			else
			{
				sl = std::make_shared<SimpleLine>(tileIndex, i);
				parentne.insert(std::make_pair(curIdKey, sl));
			}

			curIdKey = (size_t)curLine->parent;
			p = parentne.find(curIdKey);
			if (p != parentne.end())
			{
				sl->parent = p->second;
			}
			else
			{
				sl->parent = std::make_shared<SimpleLine>(tileIndex, -1);
				parentne.insert(std::make_pair(curIdKey, sl->parent));
				//sl->parent->matr = curLine->parent->matr;
			}

			int depth = curLine->getDeath();
			sl->depth = depth;
			sl->start = curLine->start;
			sl->end = curLine->end();
			sl->matrSrcSize = (int)matr.size();

			bc::barvector temp;
			for (const auto& pm : matr)
			{
				auto o = tileProv.toGlobal(pm.getX(), pm.getY());
				int x = std::min(layer.mat.wid() - 1, o.x);
				int y = std::min(layer.mat.hei() - 1, o.y);
				uint index = bc::barvalue::getStatInd(x, y);
				if (vals.find(index) != vals.end())
					continue;

				vals.insert(index);

				bc::point cp = bc::barvalue::getStatPoint(index);
				temp.push_back(bc::barvalue(cp, pm.value));

				layer.mat.set(x, y, pointCol);
				layer.setMatrPoint(x, y, curLine->getDeath(), sl);
			}

			getCountourSimple(temp, sl->matr);
		}
	}
};
//
//
//
export class CloudBarClasser : public BarClasserInnter<GeoBarCloudHolderCache, CloudItem>
{
	bc::BarcodeCreator creator;

public:
	bool useHoles = false;
	bool ignoreHeight = false;

	CloudBarClasser(const FilterInfo* filter = nullptr) : BarClasserInnter(filter)
	{ }

	void create(int tileIndex, bc::DatagridProvider* img, const bc::BarConstructor& constr, const LayerProvider& prov, RasterLineLayer* layer = nullptr)
	{
		std::unique_ptr<bc::Barcontainer> ret(creator.createBarcode(img, constr));
		//auto cloud = toCloudBarcode(ret->getItem(0), prov.getTileOffset(tileIndex);
		//cacher.save(&cloud, tileIndex);
	}

	/// read


	// -------
private:
	void classBarcodeInner(const CloudItem& baritem, int tileIndex, RasterLineLayer& layer, const LayerProvider& prov)
	{
		//bool wr = layer.extra.find("json;") != -1;
		//	bool ent = extra.indexOf("entr;") != -1;
		//	bool classif = extra.indexOf("barclass;") != -1;
		//bool showBad = true;
		//extra.indexOf("show0;") != -1;
		//bool showWhite = true; //extra.indexOf("showw;") != -1;

		auto& vec = baritem.lines;
		for (size_t i = 0; i < vec.size(); ++i)
		{
			auto* curLine = vec.at(i);

			if (filter->needSkip(curLine->lines[0]))
				continue;

			const auto& matr = curLine->matrix;

			if (matr.size() == 0)
				continue;

			Barscalar pointCol(255, 0, 0);
			int type = -1;
			{
				// It clones inside
				//type = classer.getType(curLine);
				//pointCol = colors[type + 1];

				if (type == -1)
					pointCol = colors[rand() % colors.size()];
			}

			std::unordered_set<uint> vals;
			std::shared_ptr<SimpleLine> sl = std::make_shared<SimpleLine>(tileIndex, i);
			int depth = curLine->getDeath();
			sl->depth = depth;
			sl->start = curLine->lines[0]->start;
			sl->end = curLine->lines[0]->end();
			sl->matrSrcSize = (int)matr.size();
			//		simpleHolder.push_back(sl);

			//layer.map[(size_t)sl.get()] = type;

			for (const auto& pm : matr)
			{
				const auto cp = layer.minmax(prov.toGlobalFactor(pm.getX(), pm.getY()));
				uint index = bc::barvalue::getStatInd(cp.x, cp.y);
				if (vals.find(index) != vals.end())
					continue;

				vals.insert(index);

				//bc::point p = bc::barvalue::getStatPoint(index);
				sl->matr.push_back(bc::barvalue(cp, pm.value));

				//layer.mat.set(x, y, color);
				//			outMask.set(x, y, 255);

				layer.setMatrPoint(cp.x, cp.y, curLine->getDeath(), sl);

				layer.mat.set(cp.x, cp.y, pointCol);
				if (cp.x - 1 >= 0)
				{
					layer.mat.set(cp.x - 1, cp.y, pointCol);
				}
				if (cp.x + 1 < layer.mat.wid())
				{
					layer.mat.set(cp.x + 1, cp.y, pointCol);
				}
				if (cp.y - 1 >= 0)
				{
					layer.mat.set(cp.x, cp.y - 1, pointCol);
				}
				if (cp.y + 1 < layer.mat.hei())
				{
					layer.mat.set(cp.x, cp.y + 1, pointCol);
				}
			}
		}

		// for (size_t i = 0; i < vec.size(); ++i)
		// {
		// 	auto* b = vec.at(i);
		// 	int indLocal = b->matrix[0].getIndex(layer.mat.wid());

		// 	SimpleLine* cur = layer.clickResponser[indLocal].get();
		// 	// b.
		// }
	}

	CloudItem createSplitCloudBarcode(const bc::CloudPointsBarcode::CloudPoints& cloud)
	{
		bc::CloudPointsBarcode clodCrt;
		clodCrt.useHolde = useHoles;
		std::unique_ptr<bc::Barcontainer> hold(clodCrt.createBarcode(&cloud));

		BarcodesHolder holder;
		if (cloud.points.size() == 0)
			return holder;

		bc::Baritem* main = hold->getItem(0);
		for (size_t var = 0; var < main->barlines.size(); ++var)
		{
			auto* line = main->barlines[var];
			BarcodeHolder* barhold = new BarcodeHolder();
			holder.lines.push_back(barhold);
			barhold->depth = line->getDeath();
			//barhold->matrix = std::move(line->matr);
			line->getAsListSafe(barhold->lines, true, true);
			for (auto& i : barhold->lines)
			{
				barhold->matrix.insert(barhold->matrix.begin(), i->matr.begin(), i->matr.end());
				i->matr.clear();
			}
		}

		return holder;
	}

	CloudItem toCloudBarcode(bc::Baritem* item, const bc::point& offset)
	{
		bc::CloudPointsBarcode::CloudPoints cloud;

		for (size_t var = 0; var < item->barlines.size(); ++var)
		{
			if (filter && filter->needSkip(item->barlines[var]))
			{
				continue;
			}

			auto& m = item->barlines[var]->matr[0];
			bc::point rp(m.getX() + offset.x, m.getY() + offset.y);
			cloud.points.push_back(bc::CloudPointsBarcode::CloudPoint(rp.x, rp.y, ignoreHeight ? 0 : m.value.getAvgFloat()));
		}

		return createSplitCloudBarcode(cloud);
	}
};



export class BarClassifierCache
{
public:
	void loadCategories(std::function<void(int classId, const BackString& name)> callback)
	{
		barclassificator barclas;
		BackJson loadDoc = jsonFromFile("");// Project::getPathSt(BackPath::classifier));

		JsonArray list = loadDoc["categories"];
		for (size_t i = 0; i < list.size(); i++)
		{
			JsonObject catId = list.at(i);
			callback(catId["id"], catId["name"].get<std::string>());
		}
	}

	BarCategories loadCategories()
	{
		BackPathStr path = "";// Project::getProject()->getPath(BackPath::classifier);

		BarCategories categ;
		if (!pathExists(path))
		{
			categ.value.push_back(0);
			categ.name.push_back("Bad");
			categ.value.push_back(1);
			categ.name.push_back("Kurgan");
			return categ;
		}

		auto loadColback = [&categ](int classId, const BackString& name)
		{
			categ.value.push_back(classId);
			categ.name.push_back(name);
		};
		loadCategories(loadColback);

		return categ;
	}

	void loadClasses(const BackDirStr& path, barclassificator& barclas)
	{
		barclas.udpdateClasses();
		auto& crgs = barclas.categs.value;

		for (auto categ : crgs)
		{
			BackDirStr dirl = path / intToStr(categ);
			if (!pathExists(dirl))
				continue;

			for (auto const& entry : std::filesystem::directory_iterator(dirl))
			{
				if (!entry.is_regular_file())
				{
					continue;
				}

				auto ext = entry.path().extension();
				if (ext == ".bbf")
				{
					BackPathStr filename = entry.path();
					GeoBarCloudCache reader;
					reader.openRead(filename.string());
					int ind = 0;
					std::unique_ptr<BarcodeHolder> prt(reader.load(ind));
					//barclas.addData(categ, prt->lines, true);
				}
				else if (ext == ".jpg")
				{

				}
			}
		}
	}

	void saveCategories(BarCategories& barclas)
	{
		BackPathStr path = "";// Project::getProject()->getPath(BackPath::classifier);

		JsonArray arr;
		for (size_t i = 0; i < barclas.name.size(); i++)
		{
			JsonObject catId;
			catId["id"] = barclas.value[i];
			catId["name"] = barclas.name[i];
			arr.push_back(catId);
		}

		BackJson loadDoc;
		loadDoc["categories"] = arr;
		jsonToFile(loadDoc, path);
	}

	void loadImgs(std::function<void(int classId, const BackPathStr& path)> callback, int* categorues, int size)
	{
		BackPathStr pathp = "";// Project::getPathSt(BackPath::classfiles);
		for (size_t i = 0; i < size; i++)
		{
			int categ = categorues[i];
			BackDirStr dirl = pathp / (BackPathStr)intToStr(categ);
			if (!pathExists(dirl))
				continue;

			for (auto const& entry : std::filesystem::directory_iterator(dirl))
			{
				if (!entry.is_regular_file())
				{
					continue;
				}

				BackPathStr filename = entry.path().string();
				auto ext = entry.path().extension();
				if (ext == ".jpg")
				{
					callback(categ, filename);
				}
			}
		}
	}

	void save(BarcodeHolder* curBar, int classIndex, BackImage* img)
	{
		BackPathStr path = "";// Project::getProject()->getPath(BackPath::classfiles);
		BackDirStr doorPat = path / intToStr(classIndex);
		mkDirIfNotExists(doorPat);

		std::mt19937 generator(std::random_device{}());
		std::uniform_int_distribution<int> distribution(1, 1000);
		BackString name;
		do
		{
			name = std::format("{}.bbf", distribution(generator));
		} while (pathExists(doorPat / name));
		doorPat /= name;

		GeoBarCloudCache writer;
		writer.openWrite(doorPat.string());
		writer.save(curBar, 0);

		if (img)
		{
			doorPat = doorPat.replace_extension("jpg");
			imwrite(doorPat, *img);
		}
	}
};
