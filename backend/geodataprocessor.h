#ifndef GEODATAPROCESSOR_H
#define GEODATAPROCESSOR_H

#include "barcodeCreator.h"
#include "BackImage.h"
#include "BinFile.h"
#include "Common.h"
#include <StateBinFile.h>
#include <random>
#include <vector>
#include "../frontend/Framework.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include <imgui_internal.h>

struct Cound
{
	int total = 0;
	int srcTotal = 0;
	int index = 0;
};

struct ProcPoint
{
	float x, y;
};

void getMaskRes(const MatrImg &matres, std::vector<Cound *> &veas, Cound **resmap);

void parseGeojson();
double getPsa(const bc::barvector &matr);
void getMaskRes(const MatrImg &mat, MatrImg &maskMat);
using mcountor = std::vector<uint>;
void saveAsGeojson(const bc::barlinevector &lines, const BackPathStr& savePath, ProcPoint startOffset, double coof);
void saveAllJsons(bc::barlinevector* geojson, int num, const BackDirStr& dir);
void saveJson(const std::string &text, int st);

void getCountour(const bc::barvector &points, mcountor &contur, bool aproximate = false);

using std::string;
//static int pr = 10;
//static bool normA = false;

struct BarCategories
{
	int counter;
	std::vector<int> value;
	std::vector<string> name;

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

class GeoBarCache
{
public:
	bc::Baritem* load(const BackString& str, int& index)
	{
		StateBinFile::BinStateReader reader;
		reader.open(str);
		return saveLoadBar(&reader, index);
	}
	void save(const BackString& str, int& index, bc::Baritem* item)
	{
		StateBinFile::BinStateWriter writer;
		writer.open(str);
		saveLoadBar(&writer, index, item);
	}

private:

	bc::Baritem* saveLoadBar(StateBinFile::BinState* state, int& index, bc::Baritem* rsitem = NULL)
	{
		state->beginItem();

		state->pInt(index); // Index
		int typeSize = state->pType(rsitem); // BarType

		bc::barlinevector& vec = rsitem->barlines;

		size_t linesCount = state->pArray(vec.size());
		state->beginArray(vec, linesCount);
		for (size_t i = 0; i < linesCount; ++i)
		{
			if (state->isReading())
				vec[i] = new bc::barline();

			bc::barline* line = vec[i];

			uint id = state->pInt(i);
			line->start = state->pBarscalar(line->start);
			line->m_end = (state->pBarscalar(line->end()));

			//act matrSize = state->pFixedArray(line->matr, 4 + typeSize);
			uint matrSize = state->pArray(line->matr.size());
			state->beginArray(line->matr, matrSize);
			for (uint j = 0; j < matrSize; ++j)
			{
				bc::barvalue& v = line->matr[j];
				v.index = state->pInt(v.index);
				v.value = state->pBarscalar(v.value);
			}
		}
		state->endItem();

		return rsitem;
	}
};

using CloudBarcodeHolder = BarcodeHolder;
using CloudItem = BarcodesHolder;
class GeoBarHolderCache
{
	bc::BarcodeCreator creator;
	std::unique_ptr<StateBinFile::BinState> state;
public:

	bool canRead()
	{
		return !state->ended();
	}
	void openRead(const BackPathStr& str)
	{
		state.reset(new StateBinFile::BinStateReader());
		if (!state->open(str.string()))
			throw;
	}
	void openWrite(const BackPathStr& str)
	{
		state.reset(new StateBinFile::BinStateWriter());
		if (!state->open(str.string()))
			throw;
	}

	void openRead();
	void openWrite();

	BarcodeHolder* load(int& index)
	{
		return saveLoadBar(index, new BarcodeHolder());
	}

	CloudItem* loadCloudItemByIndex(int index)
	{
		dynamic_cast<StateBinFile::BinStateReader*>(state.get())->moveIndex(index);
		return saveLoadBars(index, new CloudItem());
	}

	CloudItem* loadCloudItem(int& index)
	{
		return saveLoadBars(index, new CloudItem());
	}

	void save(int index, CloudBarcodeHolder* item)
	{
		saveLoadBar(index, item);
	}

	void save(int index, CloudItem* item)
	{
		saveLoadBars(index, item);
	}

	bc::Baritem* create(bc::DatagridProvider* prov, const bc::BarConstructor& constr)
	{
		std::unique_ptr<bc::Barcontainer> ret(creator.createBarcode(prov, constr));
		return ret->exractItem(0);
	}

	CloudItem createSplitCloudBarcode(const bc::CloudPointsBarcode::CloundPoints& cloud)
	{
		bc::CloudPointsBarcode clodCrt;
		clodCrt.useHolde = true;
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

			barhold->matrix = std::move(line->matr);
			line->getAsListSafe(barhold->lines, true, false);
		}

		return holder;
	}

	CloudBarcodeHolder createSingleCloudBarcode(const bc::CloudPointsBarcode::CloundPoints& cloud)
	{
		bc::CloudPointsBarcode clodCrt;
		std::unique_ptr<bc::Barcontainer> hold(clodCrt.createBarcode(&cloud));

		CloudBarcodeHolder holder;
		if (cloud.points.size() == 0)
			return holder;

		bc::Baritem* main = hold->getItem(0);
		for (size_t var = 0; var < main->barlines.size(); ++var)
		{
			auto* line = main->barlines[var];
			holder.lines.push_back(line);
			holder.matrix.insert(holder.matrix.begin(), line->matr.begin(), line->matr.end());
		}

		return holder;
	}


	CloudItem createCloudBarcode(bc::DatagridProvider* prov, const bc::BarConstructor& constr)
	{
		std::unique_ptr<bc::Baritem> item(create(prov, constr));

		bc::CloudPointsBarcode::CloundPoints cloud;
		for (size_t var = 0; var < item->barlines.size(); ++var)
		{
			auto& m = item->barlines[var]->matr[0];
			bc::point rp(m.getX(), m.getY());
			cloud.points.push_back(bc::CloudPointsBarcode::CloundPoint(rp.x, rp.y, m.value.getAvgFloat()));
		}

		return createSplitCloudBarcode(cloud);
	}


	BarcodeHolder createSingleCloudBarcode(bc::DatagridProvider* prov, const bc::BarConstructor& constr)
	{
		std::unique_ptr<bc::Baritem> item(create(prov, constr));

		bc::CloudPointsBarcode::CloundPoints cloud;
		for (size_t var = 0; var < item->barlines.size(); ++var)
		{
			auto& m = item->barlines[var]->matr[0];
			bc::point rp(m.getX(), m.getY());
			cloud.points.push_back(bc::CloudPointsBarcode::CloundPoint(rp.x, rp.y, m.value.getAvgFloat()));
		}

		return createSingleCloudBarcode(cloud);
	}


	CloudItem toCloundBarcode(bc::Baritem* item, bc::point offset, const FilterInfo& info)
	{
		bc::CloudPointsBarcode::CloundPoints cloud;
		for (size_t var = 0; var < item->barlines.size(); ++var)
		{
			if (info.needSkip(item->barlines[var]->len()))
			{
				continue;
			}

			auto& m = item->barlines[var]->matr[0];
			bc::point rp(m.getX() + offset.x, m.getY() + offset.y);
			cloud.points.push_back(bc::CloudPointsBarcode::CloundPoint(rp.x, rp.y, m.value.getAvgFloat()));
		}

		return createSplitCloudBarcode(cloud);
	}


private:

	BarcodeHolder* saveLoadBarBody(BarcodeHolder* rsitem = NULL)
	{
		bc::barlinevector& vec = rsitem->lines;

		size_t linesCount = state->pArray(vec.size());
		state->beginArray(vec, linesCount);
		for (size_t i = 0; i < linesCount; ++i)
		{
			if (state->isReading())
				vec[i] = new bc::barline();

			bc::barline* line = vec[i];

			uint id = state->pInt(i);
			line->start = state->pBarscalar(line->start);
			line->m_end = (state->pBarscalar(line->end()));
		}

		//act matrSize = state->pFixedArray(line->matr, 4 + typeSize);
		uint matrSize = state->pArray(rsitem->matrix.size());
		state->beginArray(rsitem->matrix, matrSize);
		for (uint j = 0; j < matrSize; ++j)
		{
			bc::barvalue& v = rsitem->matrix[j];
			v.index = state->pInt(v.index);
			v.value = state->pBarscalar(v.value);
		}

		return rsitem;
	}

	BarcodeHolder* saveLoadBar(int& index, BarcodeHolder* rsitem = NULL)
	{
		state->beginItem();
		state->pInt(index);
		state->pType(rsitem->matrix.size() > 0 ? rsitem->matrix[0].value.type : BarType::FLOAT32_1);

		rsitem = saveLoadBarBody(rsitem);
		state->endItem();
		return rsitem;
	}

	CloudItem* saveLoadBars(int& index, CloudItem* rsitem = NULL)
	{
		state->beginItem();
		index = state->pInt(index);
		auto& lines = rsitem->lines;
		state->pType(lines.size() > 0 && lines[0]->matrix.size() > 0 ? lines[0]->matrix[0].value.type : BarType::FLOAT32_1);

		uint linesSize = state->pArray(rsitem->lines.size());
		state->beginArray(rsitem->lines, linesSize);
		for (size_t i = 0; i < linesSize; i++)
		{
			if (state->isReading())
				rsitem->lines[i] = new BarcodeHolder();

			saveLoadBarBody(rsitem->lines[i]);
		}
		state->endItem();
		return rsitem;
	}
};

class barclassificator
{
public:
	BarCategories categs;
	std::vector < bc::Barcontainer> classes;

	void addData(int classInd, bc::barlinevector &cont, const bool move = true)
	{
		bc::Baritem *item = new bc::Baritem();
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

private:
	Barscalar asScalar(const BackJson &arr)
	{
		return Barscalar(arr.at(0).get<double>(), arr.at(1).get<double>(), arr.at(2).get<double>());
	}

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

	//BarcodesHolder toHoldes(BarcodesHolder& lines, MatrImg& mat, bc::point offset, float factor)
	//{
	//	bc::CloudPointsBarcode::CloundPoints cloud;
	//	for (size_t var = 0; var < lines.lines.size(); ++var)
	//	{
	//		//if (needSkip(lines.lines[var]->lines[0]->len()))
	//		//{
	//		//	continue;
	//		//}

	//		auto& m = lines.lines[var]->matrix[0];
	//		bc::point rp(m.getX() + offset.x, m.getY() + offset.y);
	//		bc::point cp = rp * factor;
	//		mat.set(cp.x, cp.y, Barscalar(255, 0, 0));
	//		if (cp.x - 1 >= 0)
	//		{
	//			mat.set(cp.x - 1, cp.y, Barscalar(255, 0, 0));
	//		}
	//		if (cp.x + 1 < mat.wid())
	//		{
	//			mat.set(cp.x + 1, cp.y, Barscalar(255, 0, 0));
	//		}
	//		if (cp.y - 1 >= 0)
	//		{
	//			mat.set(cp.x, cp.y - 1, Barscalar(255, 0, 0));
	//		}
	//		if (cp.y + 1 < mat.wid())
	//		{
	//			mat.set(cp.x, cp.y + 1, Barscalar(255, 0, 0));
	//		}
	//		assert(cp.x >= 0);
	//		assert(cp.y >= 0);
	//		cloud.points.push_back(bc::CloudPointsBarcode::CloundPoint(rp.x, rp.y, m.value.getAvgFloat()));
	//	}

	//	return toHoldes(cloud);
	//}

	//BarcodesHolder toHoldes(const bc::CloudPointsBarcode::CloundPoints& cloud)
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


public:
	//void addClass(const BackPathStr& binFile, int classInd)
	//{
	//	BackString val;

	//	std::ifstream file(path.string());
	//	BackJson jsonDocument = BackJson::parse(file);

	//	// Из которого выделяем объект в текущий рабочий QJsonObject
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

void removeLast(int classInd)
{
	classes[classInd].remoeLast();
}

//	int getType(bc::barlinevector &bar0lines)
int getType(const BarcodeHolder* bar0)
{
	auto cp = bc::CompireStrategy::CommonToLen;
	float res = 0;

	//		bc::Baritem *bar0 = new bc::Baritem();
	//		bar0->barlines = bar0lines;
	bc::Baritem newOne;
	bar0->cloneLines(newOne.barlines);
	//		newOne.shdowCopy = true;
	newOne.relen();

	int maxInd = -1;
	float maxP = res;
	for (size_t i = 0; i < classes.size(); i++)
	{
		float ps = classes[i].compireBest(&newOne, cp);
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
};


class BarClassifierCache
{
public:
	BarCategories loadCategories();

	void saveCategories(BarCategories& barclas);

	void loadCategories(std::function<void(int classId, const BackString& name)> callback);

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
					GeoBarHolderCache reader;
					reader.openRead(filename.string());
					int ind = 0;
					std::unique_ptr<BarcodeHolder> prt(reader.load(ind));
					barclas.addData(categ, prt->lines, true);
				}
				else if (ext == ".jpg")
				{

				}
			}
		}
	}

	void loadImgs(std::function<void(int classId, const BackPathStr& path)> callback, int* categorues, int size);

	void save(BarcodeHolder* curBar, int classIndex, BackImage* img = nullptr);
};



inline void testC()
{
	int a4[] = {2,  25, 255, 3,	22, 255, 3,	 23, 255, 3,  24, 255, 3,  25, 255, 4,	19, 255, 4,	 20, 255, 4,  21, 255, 4,  22, 255, 4,	23, 255, 4,	 24, 255, 4,  25, 255, 5,  19, 255, 5,	20, 255, 5,	 21, 255, 5,  22, 255, 5,  23, 255, 5,	24, 255, 5,	 25, 255, 5,  26, 255, 6,  19, 255,
			   6,  20, 255, 6,	21, 255, 6,	 22, 255, 6,  23, 255, 6,  24, 255, 6,	25, 255, 6,	 26, 255, 7,  18, 255, 7,  19, 255, 7,	20, 255, 7,	 21, 255, 7,  22, 255, 7,  23, 255, 7,	24, 255, 7,	 25, 255, 7,  26, 255, 7,  27, 255, 8,	12, 255, 8,	 13, 255, 8,  18, 255, 8,  19, 255,
			   8,  21, 255, 8,	22, 255, 8,	 23, 255, 8,  24, 255, 8,  25, 255, 8,	26, 255, 8,	 27, 255, 9,  11, 255, 9,  12, 255, 9,	13, 255, 9,	 17, 255, 9,  18, 255, 9,  19, 255, 9,	20, 255, 9,	 21, 255, 9,  22, 255, 9,  23, 255, 9,	24, 255, 9,	 25, 255, 9,  26, 255, 9,  27, 255,
			   10, 11, 255, 10, 12, 255, 10, 13, 255, 10, 14, 255, 10, 15, 255, 10, 16, 255, 10, 17, 255, 10, 18, 255, 10, 19, 255, 10, 20, 255, 10, 21, 255, 10, 22, 255, 10, 23, 255, 10, 24, 255, 10, 25, 255, 10, 26, 255, 10, 27, 255, 10, 28, 255, 11, 11, 255, 11, 12, 255, 11, 13, 255,
			   11, 14, 255, 11, 15, 255, 11, 16, 255, 11, 17, 255, 11, 18, 255, 11, 19, 255, 11, 20, 255, 11, 21, 255, 11, 22, 255, 11, 23, 255, 11, 24, 255, 11, 28, 255, 12, 8,  255, 12, 11, 255, 12, 12, 255, 12, 13, 255, 12, 14, 255, 12, 15, 255, 12, 16, 255, 12, 18, 255, 12, 19, 255,
			   12, 20, 255, 12, 21, 255, 12, 22, 255, 12, 23, 255, 12, 24, 255, 12, 25, 255, 12, 26, 255, 12, 28, 255, 13, 8,  255, 13, 9,	255, 13, 10, 255, 13, 11, 255, 13, 12, 255, 13, 13, 255, 13, 14, 255, 13, 15, 255, 13, 16, 255, 13, 17, 255, 13, 18, 255, 13, 19, 255, 13, 20, 255,
			   13, 21, 255, 13, 22, 255, 13, 23, 255, 13, 24, 255, 13, 25, 255, 13, 26, 255, 13, 28, 255, 14, 9,  255, 14, 10, 255, 14, 11, 255, 14, 12, 255, 14, 13, 255, 14, 14, 255, 14, 15, 255, 14, 16, 255, 14, 17, 255, 14, 18, 255, 14, 19, 255, 14, 20, 255, 14, 21, 255, 14, 22, 255,
			   14, 23, 255, 14, 24, 255, 14, 25, 255, 14, 26, 255, 14, 28, 255, 15, 10, 255, 15, 11, 255, 15, 12, 255, 15, 13, 255, 15, 14, 255, 15, 15, 255, 15, 16, 255, 15, 17, 255, 15, 18, 255, 15, 19, 255, 15, 20, 255, 15, 21, 255, 15, 22, 255, 15, 23, 255, 15, 24, 255, 15, 25, 255,
			   15, 26, 255, 15, 27, 255, 16, 10, 255, 16, 11, 255, 16, 12, 255, 16, 13, 255, 16, 14, 255, 16, 15, 255, 16, 16, 255, 16, 17, 255, 16, 18, 255, 16, 19, 255, 16, 20, 255, 16, 21, 255, 16, 22, 255, 16, 24, 255, 16, 25, 255, 16, 26, 255, 16, 27, 255, 17, 10, 255, 17, 11, 255,
			   17, 12, 255, 17, 13, 255, 17, 14, 255, 17, 15, 255, 17, 16, 255, 17, 17, 255, 17, 18, 255, 17, 19, 255, 17, 20, 255, 17, 21, 255, 17, 22, 255, 17, 23, 255, 17, 24, 255, 17, 25, 255, 17, 26, 255, 18, 9,  255, 18, 10, 255, 18, 11, 255, 18, 12, 255, 18, 13, 255, 18, 14, 255,
			   18, 15, 255, 18, 16, 255, 18, 18, 255, 18, 19, 255, 18, 21, 255, 18, 22, 255, 18, 23, 255, 18, 24, 255, 18, 26, 255, 19, 8,	255, 19, 9,	 255, 19, 10, 255, 19, 11, 255, 19, 20, 255, 19, 22, 255, 19, 23, 255, 19, 27, 255, 20, 7,	255, 20, 8,	 255, 20, 9,  255, 20, 10, 255,
			   20, 11, 255, 20, 21, 255, 20, 22, 255, 20, 23, 255, 20, 26, 255, 20, 27, 255, 20, 28, 255, 21, 7,  255, 21, 8,  255, 21, 9,	255, 21, 10, 255, 21, 21, 255, 21, 22, 255, 21, 26, 255, 21, 27, 255, 22, 7,  255, 22, 8,  255, 22, 9,	255, 22, 20, 255, 22, 22, 255, 22, 23, 255,
			   22, 24, 255, 22, 26, 255, 22, 27, 255, 22, 28, 255, 23, 7,  255, 23, 21, 255, 23, 22, 255, 23, 23, 255, 23, 24, 255, 23, 26, 255, 23, 27, 255, 23, 28, 255, 24, 21, 255, 24, 22, 255, 24, 23, 255, 24, 24, 255, 24, 25, 255, 24, 26, 255, 24, 27, 255, 25, 21, 255, 25, 22, 255,
			   25, 23, 255, 25, 24, 255, 25, 26, 255, 25, 27, 255, 25, 29, 255, 26, 21, 255, 26, 22, 255, 26, 23, 255, 26, 24, 255, 26, 25, 255, 26, 26, 255, 26, 27, 255, 26, 28, 255, 27, 21, 255, 27, 22, 255, 27, 23, 255, 27, 24, 255, 27, 26, 255, 27, 27, 255, 27, 28, 255, 28, 21, 255,
			   28, 22, 255, 28, 23, 255, 28, 24, 255, 28, 25, 255, 28, 26, 255, 28, 27, 255, 28, 28, 255, 29, 21, 255, 29, 22, 255, 29, 23, 255, 29, 24, 255, 29, 25, 255, 29, 26, 255, 29, 27, 255, 29, 28, 255, 30, 21, 255, 30, 22, 255, 30, 23, 255, 30, 24, 255, 30, 25, 255, 30, 26, 255,
			   30, 27, 255, 30, 28, 255, 31, 20, 255, 31, 21, 255, 31, 22, 255, 31, 25, 255, 31, 26, 255, 31, 27, 255, 31, 28, 255, 32, 19, 255, 32, 20, 255, 32, 21, 255, 32, 22, 255, 32, 25, 255, 32, 26, 255, 32, 27, 255, 32, 28, 255, 33, 26, 255};

		int a1[] = {2, 7, 255, 3, 6, 255, 3, 7,	255, 3, 8, 255, 3, 9, 255, 3, 10, 255, 3, 11, 255, 4, 5, 255, 4, 6, 255, 4, 7,	255, 4,	 8, 255, 4,	 9, 255, 4,	 10, 255, 4,  11, 255, 5,  5, 255, 5,  6,  255, 5,	7, 255,
				   5, 8, 255, 5, 9, 255, 5, 10, 255, 6, 4, 255, 6, 5, 255, 6, 6,  255, 6, 7,  255, 6, 8, 255, 6, 9, 255, 6, 10, 255, 7,	 4, 255, 7,	 5, 255, 7,	 6,	 255, 7,  7,  255, 7,  8, 255, 7,  10, 255, 8,	3, 255,
				   8, 4, 255, 8, 5, 255, 8, 6,	255, 8, 7, 255, 8, 8, 255, 9, 4,  255, 9, 5,  255, 9, 6, 255, 9, 7, 255, 9, 8,	255, 10, 5, 255, 10, 6, 255, 10, 7,	 255, 11, 4,  255, 11, 7, 255, 12, 3,  255, 13, 2, 255};

	int ar[] = {1,  10, 255, 2,	9, 255, 2,	11, 255, 3,	 8,	 255, 3,  9,  255, 3,  10, 255, 3,	12, 255, 4,	 8, 255, 4,	 9, 255, 4,	 10, 255, 4,  11, 255, 4,  12, 255, 5,	6,	255, 5,	 7, 255, 5,	 8, 255, 5,	 9, 255, 5,	 10, 255, 5,  11, 255, 5,  12, 255, 6,	6,	255,
			   6,  7,  255, 6,	8, 255, 6,	9,	255, 6,	 10, 255, 6,  11, 255, 6,  12, 255, 7,	5,	255, 7,	 6, 255, 7,	 7, 255, 7,	 9,	 255, 7,  10, 255, 7,  11, 255, 7,	12, 255, 8,	 4, 255, 8,	 5, 255, 8,	 6, 255, 8,	 7,	 255, 8,  8,  255, 8,  9,  255, 8,	10, 255,
			   9,  3,  255, 9,	4, 255, 9,	5,	255, 9,	 7,	 255, 9,  8,  255, 9,  9,  255, 10, 3,	255, 10, 4, 255, 10, 5, 255, 10, 6,	 255, 10, 7,  255, 10, 8,  255, 10, 9,	255, 11, 4, 255, 11, 5, 255, 11, 6, 255, 11, 7,	 255, 11, 8,  255, 12, 3,  255, 12, 4,	255,
			   12, 5,  255, 12, 6, 255, 13, 1,	255, 13, 3,	 255, 13, 4,  255, 13, 5,  255, 13, 6,	255, 13, 7, 255, 14, 2, 255, 14, 3,	 255, 14, 4,  255, 14, 5,  255, 14, 6,	255, 14, 7, 255, 15, 3, 255, 15, 4, 255, 15, 5,	 255, 16, 3,  255, 16, 4,  255, 16, 5,	255};

	int ar2[] = {1, 19, 255, 1, 20, 255, 1, 21, 255, 2, 19, 255, 2, 20, 255, 2, 21, 255, 2, 22, 255, 2, 23, 255, 3, 16, 255, 3, 17, 255, 3, 18, 255, 3, 19, 255, 3, 20, 255, 3, 21, 255, 3,	 22, 255, 3,  23, 255, 3,  24, 255, 4,	16, 255, 4,	 17, 255, 4,  18, 255, 4, 19, 255,
	 4, 20, 255, 4, 21, 255, 4, 22, 255, 4, 23, 255, 4, 24, 255, 5, 17, 255, 5, 20, 255, 5, 21, 255, 5, 22, 255, 5, 23, 255, 5, 24, 255, 6, 20, 255, 6, 21, 255, 6, 22, 255, 6,	 23, 255, 6,  24, 255, 6,  25, 255, 7,	20, 255, 7,	 21, 255, 7,  22, 255, 7, 23, 255,
	 7, 24, 255, 7, 25, 255, 8, 20, 255, 8, 21, 255, 8, 22, 255, 8, 23, 255, 8, 24, 255, 8, 25, 255, 9, 20, 255, 9, 21, 255, 9, 22, 255, 9, 23, 255, 9, 25, 255, 9, 26, 255, 10, 23, 255, 10, 25, 255, 11, 24, 255, 12, 23, 255, 12, 24, 255, 13, 23, 255};

	bc::barvector points;
	auto& a = ar2;
	for (int i = 0; i < sizeof(a) / 4; i += 3)
	{
		points.push_back(bc::barvalue(a[i + 1], a[i], a[i + 2]));
	}

	mcountor cont;
	getCountour(points, cont, false);
	std::cout << "Good" << std::endl;
}

void getSet(string path, barclassificator& data, char diff = '0');

#endif // GEODATAPROCESSOR_H
