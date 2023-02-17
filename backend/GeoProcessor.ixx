module;

#include "barcodeCreator.h"
#include "BackImage.h"
#include <random>
#include <vector>
#include "../frontend/Framework.h"
#include "Common.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include <imgui_internal.h>


export module GeoprocessorModule;

import CacheFilesModule;


export struct Cound
{
	int total = 0;
	int srcTotal = 0;
	int index = 0;
};

export struct ProcPoint
{
	float x, y;
};

export void getMaskRes(const MatrImg& matres, std::vector<Cound*>& veas, Cound** resmap);

export void parseGeojson();
export double getPsa(const bc::barvector& matr);
export void getMaskRes(const MatrImg& mat, MatrImg& maskMat);
export using mcountor = std::vector<uint>;
export void saveAsGeojson(const bc::barlinevector& lines, const BackPathStr& savePath, ProcPoint startOffset, double coof);
export void saveJson(const std::string& text, int st);

void getCountour(const bc::barvector& points, mcountor& contur, bool aproximate = false);

using std::string;
//static int pr = 10;
//static bool normA = false;


export class CloudBarcodeCreateHelper
{
	bc::BarcodeCreator creator;

public:
	static bool useHols;
	static bool ignoreHeight;

	CloudBarcodeCreateHelper()
	{ }

	bc::Baritem* create(bc::DatagridProvider* prov, const bc::BarConstructor& constr)
	{
		std::unique_ptr<bc::Barcontainer> ret(creator.createBarcode(prov, constr));
		return ret->exractItem(0);
	}

	CloudItem createSplitCloudBarcode(const bc::CloudPointsBarcode::CloudPoints& cloud)
	{
		bc::CloudPointsBarcode clodCrt;
		clodCrt.useHolde = useHols;
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


	CloudItem toCloudBarcode(bc::Baritem* item, bc::point offset, const FilterInfo& info)
	{
		bc::CloudPointsBarcode::CloudPoints cloud;
		for (size_t var = 0; var < item->barlines.size(); ++var)
		{
			if (info.needSkip(item->barlines[var]))
			{
				continue;
			}

			auto& m = item->barlines[var]->matr[0];
			bc::point rp(m.getX() + offset.x, m.getY() + offset.y);
			cloud.points.push_back(bc::CloudPointsBarcode::CloudPoint(rp.x, rp.y, ignoreHeight ? 0 : m.value.getAvgFloat()));
		}

		return createSplitCloudBarcode(cloud);
	}

	// CloudBarcodeHolder createSingleCloudBarcode(const bc::CloudPointsBarcode::CloudPoints& cloud)
	// {
	// 	bc::CloudPointsBarcode clodCrt;
	// 	std::unique_ptr<bc::Barcontainer> hold(clodCrt.createBarcode(&cloud));

	// 	CloudBarcodeHolder holder;
	// 	if (cloud.points.size() == 0)
	// 		return holder;

	// 	bc::Baritem* main = hold->getItem(0);
	// 	for (size_t var = 0; var < main->barlines.size(); ++var)
	// 	{
	// 		auto* line = main->barlines[var];
	// 		holder.lines.push_back(line);
	// 		holder.matrix.insert(holder.matrix.begin(), line->matr.begin(), line->matr.end());
	// 	}

	// 	return holder;
	// }


	// CloudItem createCloudBarcode(bc::DatagridProvider* prov, const bc::BarConstructor& constr)
	// {
	// 	std::unique_ptr<bc::Baritem> item(create(prov, constr));

	// 	bc::CloudPointsBarcode::CloudPoints cloud;
	// 	for (size_t var = 0; var < item->barlines.size(); ++var)
	// 	{
	// 		auto& m = item->barlines[var]->matr[0];
	// 		bc::point rp(m.getX(), m.getY());
	// 		cloud.points.push_back(bc::CloudPointsBarcode::CloudPoints(rp.x, rp.y, m.value.getAvgFloat()));
	// 	}

	// 	return createSplitCloudBarcode(cloud);
	// }


	// BarcodeHolder createSingleCloudBarcode(bc::DatagridProvider* prov, const bc::BarConstructor& constr)
	// {
	// 	std::unique_ptr<bc::Baritem> item(create(prov, constr));

	// 	bc::CloudPointsBarcode::CloudPoints cloud;
	// 	for (size_t var = 0; var < item->barlines.size(); ++var)
	// 	{
	// 		auto& m = item->barlines[var]->matr[0];
	// 		bc::point rp(m.getX(), m.getY());
	// 		cloud.points.push_back(bc::CloudPointsBarcode::CloudPoints(rp.x, rp.y, m.value.getAvgFloat()));
	// 	}

	// 	return createSingleCloudBarcode(cloud);
	// }


};


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
private:
	Barscalar asScalar(const BackJson& arr)
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
	//	bc::CloudPointsBarcode::CloudPoints cloud;
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
	//		cloud.points.push_back(bc::CloudPointsBarcode::CloudPoints(rp.x, rp.y, m.value.getAvgFloat()));
	//	}

	//	return toHoldes(cloud);
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
};


export class BarClassifierCache
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
					GeoBarCloudCache reader;
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
	int a4[] = { 2,  25, 255, 3,	22, 255, 3,	 23, 255, 3,  24, 255, 3,  25, 255, 4,	19, 255, 4,	 20, 255, 4,  21, 255, 4,  22, 255, 4,	23, 255, 4,	 24, 255, 4,  25, 255, 5,  19, 255, 5,	20, 255, 5,	 21, 255, 5,  22, 255, 5,  23, 255, 5,	24, 255, 5,	 25, 255, 5,  26, 255, 6,  19, 255,
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
				30, 27, 255, 30, 28, 255, 31, 20, 255, 31, 21, 255, 31, 22, 255, 31, 25, 255, 31, 26, 255, 31, 27, 255, 31, 28, 255, 32, 19, 255, 32, 20, 255, 32, 21, 255, 32, 22, 255, 32, 25, 255, 32, 26, 255, 32, 27, 255, 32, 28, 255, 33, 26, 255 };

	int a1[] = { 2, 7, 255, 3, 6, 255, 3, 7,	255, 3, 8, 255, 3, 9, 255, 3, 10, 255, 3, 11, 255, 4, 5, 255, 4, 6, 255, 4, 7,	255, 4,	 8, 255, 4,	 9, 255, 4,	 10, 255, 4,  11, 255, 5,  5, 255, 5,  6,  255, 5,	7, 255,
				5, 8, 255, 5, 9, 255, 5, 10, 255, 6, 4, 255, 6, 5, 255, 6, 6,  255, 6, 7,  255, 6, 8, 255, 6, 9, 255, 6, 10, 255, 7,	 4, 255, 7,	 5, 255, 7,	 6,	 255, 7,  7,  255, 7,  8, 255, 7,  10, 255, 8,	3, 255,
				8, 4, 255, 8, 5, 255, 8, 6,	255, 8, 7, 255, 8, 8, 255, 9, 4,  255, 9, 5,  255, 9, 6, 255, 9, 7, 255, 9, 8,	255, 10, 5, 255, 10, 6, 255, 10, 7,	 255, 11, 4,  255, 11, 7, 255, 12, 3,  255, 13, 2, 255 };

	int ar[] = { 1,  10, 255, 2,	9, 255, 2,	11, 255, 3,	 8,	 255, 3,  9,  255, 3,  10, 255, 3,	12, 255, 4,	 8, 255, 4,	 9, 255, 4,	 10, 255, 4,  11, 255, 4,  12, 255, 5,	6,	255, 5,	 7, 255, 5,	 8, 255, 5,	 9, 255, 5,	 10, 255, 5,  11, 255, 5,  12, 255, 6,	6,	255,
				6,  7,  255, 6,	8, 255, 6,	9,	255, 6,	 10, 255, 6,  11, 255, 6,  12, 255, 7,	5,	255, 7,	 6, 255, 7,	 7, 255, 7,	 9,	 255, 7,  10, 255, 7,  11, 255, 7,	12, 255, 8,	 4, 255, 8,	 5, 255, 8,	 6, 255, 8,	 7,	 255, 8,  8,  255, 8,  9,  255, 8,	10, 255,
				9,  3,  255, 9,	4, 255, 9,	5,	255, 9,	 7,	 255, 9,  8,  255, 9,  9,  255, 10, 3,	255, 10, 4, 255, 10, 5, 255, 10, 6,	 255, 10, 7,  255, 10, 8,  255, 10, 9,	255, 11, 4, 255, 11, 5, 255, 11, 6, 255, 11, 7,	 255, 11, 8,  255, 12, 3,  255, 12, 4,	255,
				12, 5,  255, 12, 6, 255, 13, 1,	255, 13, 3,	 255, 13, 4,  255, 13, 5,  255, 13, 6,	255, 13, 7, 255, 14, 2, 255, 14, 3,	 255, 14, 4,  255, 14, 5,  255, 14, 6,	255, 14, 7, 255, 15, 3, 255, 15, 4, 255, 15, 5,	 255, 16, 3,  255, 16, 4,  255, 16, 5,	255 };

	int ar2[] = { 1, 19, 255, 1, 20, 255, 1, 21, 255, 2, 19, 255, 2, 20, 255, 2, 21, 255, 2, 22, 255, 2, 23, 255, 3, 16, 255, 3, 17, 255, 3, 18, 255, 3, 19, 255, 3, 20, 255, 3, 21, 255, 3,	 22, 255, 3,  23, 255, 3,  24, 255, 4,	16, 255, 4,	 17, 255, 4,  18, 255, 4, 19, 255,
		4, 20, 255, 4, 21, 255, 4, 22, 255, 4, 23, 255, 4, 24, 255, 5, 17, 255, 5, 20, 255, 5, 21, 255, 5, 22, 255, 5, 23, 255, 5, 24, 255, 6, 20, 255, 6, 21, 255, 6, 22, 255, 6,	 23, 255, 6,  24, 255, 6,  25, 255, 7,	20, 255, 7,	 21, 255, 7,  22, 255, 7, 23, 255,
		7, 24, 255, 7, 25, 255, 8, 20, 255, 8, 21, 255, 8, 22, 255, 8, 23, 255, 8, 24, 255, 8, 25, 255, 9, 20, 255, 9, 21, 255, 9, 22, 255, 9, 23, 255, 9, 25, 255, 9, 26, 255, 10, 23, 255, 10, 25, 255, 11, 24, 255, 12, 23, 255, 12, 24, 255, 13, 23, 255 };

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

#include <stack>
#include <ranges>

bool CloudBarcodeCreateHelper::useHols = true;
bool CloudBarcodeCreateHelper::ignoreHeight = false;

double getPsa(const bc::barvector& matr)
{
	std::unordered_map<uint, bool> map;

	int minX = 10000, maxX = 0, minY = 1000, maxY = 0;
	for (const auto& pm : matr)
	{
		map[pm.index] = true;
		int xa = pm.getX();
		int ya = pm.getY();
		if (xa > maxX)
			maxX = xa;
		if (xa < minX)
			minX = xa;

		if (ya > maxY)
			maxY = ya;
		if (ya < minY)
			minY = ya;
	}

	double entr = 0;
	bool foundPrev = map.find(bc::barvalue::getStatInd(minX, minY)) != map.end();
	for (int x = minX; x < maxX; ++x)
	{
		double locEntr = 1.0;
		for (int y = minY; y < maxY; ++y)
		{
			bool foundCur = map.find(bc::barvalue::getStatInd(x, y)) != map.end();

			locEntr *= 0.5;
			if (foundPrev != foundCur)
			{
				entr += locEntr;
				locEntr = 1.0;
			}
		}

		if (locEntr != 1.0)
			entr += locEntr;
	}

	for (int y = minY; y < maxY; ++y)
	{
		double locEntr = 1.0;
		for (int x = minX; x < maxX; ++x)
		{
			bool foundCur = map.find(bc::barvalue::getStatInd(x, y)) != map.end();
			locEntr *= 0.5;
			if (foundPrev != foundCur)
			{
				entr += locEntr;
				locEntr = 1.0;
			}
		}

		if (locEntr != 1.0)
			entr += locEntr;
	}

	return entr;
}



void getMaskRes(const MatrImg& matres, std::vector<Cound*>& veas, Cound** resmap)
{
	if (veas.size() == 0)
		return;

	for (int i = 0, total = veas.size(); i < total; ++i)
		veas[i]->total = 0;

	int bad = 0;
	for (int y = 0; y < matres.hei(); ++y)
	{
		for (int x = 0; x < matres.wid(); ++x)
		{
			int ind = y * matres.wid() + x;
			auto a = matres.get(x, y);
			Cound* b = resmap[ind];
			if (b)
			{
				if (a > 0)
					b->total++;
			}
			else if (a > 0)
				++bad;
		}
	}

	int fdo = 0;
	float proc = 0;
	for (int i = 0, total = veas.size(); i < total; ++i)
	{
		auto& d = veas[i];
		float p = static_cast<float>(d->total) / d->srcTotal;
		if (p >= 0.6)
			p = 1.0;

		proc += p;
		//*static_cast<float>(d->srcTotal) / matres.length();
		d->total = 0;
		float sadfe = 100.f * static_cast<float>(d->srcTotal) / matres.length();
		if (sadfe > 0.015)
			++fdo;
	}
	proc /= fdo;

	float ew = static_cast<float>(bad) / matres.length();
	std::cout << "Res src: " << proc << std::endl;
	std::cout << "Res bad: " << proc - ew << std::endl;
}

void getMaskRes(const MatrImg& mat, MatrImg& maskMat)
{
	if (maskMat.wid() == 1)
		return;

	int edd = 0;
	int edd1 = 0;
	int edd2 = 0;
	int tot1 = 0;
	int tot2 = 0;

	for (int y = 0; y < mat.hei(); ++y)
	{
		for (int x = 0; x < mat.wid(); ++x)
		{
			auto a = mat.get(x, y);
			auto b = maskMat.get(x, y);
			if (b > 0)
				++tot1;
			else if (b == 0)
				++tot2;

			if ((a > 0 && b > 0))
			{
				++edd;
				++edd1;
			}
			else if (a == 0 && b == 0)
			{
				++edd;
				++edd2;
			}
			else
			{
				--edd;
			}
		}
	}
	float proc = (static_cast<float>(mat.length()) + static_cast<float>(edd)) / (mat.length() * 2);
	std::cout << "Res counter: " << edd << "; porc: " << proc << std::endl;
	std::cout << "Res Hit: " << edd1 << "; porc: " << static_cast<float>(edd1) / tot1 << std::endl;
	std::cout << "Res Clear: " << edd2 << "; porc: " << static_cast<float>(edd2) / tot2 << std::endl;
}

#define DICT_EXISTS(DICT, X, Y) (DICT.find(bc::barvalue::getStatInd(X, Y)) != DICT.end())

class MapCountur
{
	int x = 0, y = 0;
	int stIndex = 0;
	mcountor& contur;
	std::unordered_map<uint, bool> points;

	enum StartPos : char { LeftMid = 0, LeftTop = 1, TopMid = 2, RigthTop = 3, RigthMid = 4, RigthBottom = 5, BottomMid = 6, LeftBottom = 7 };

	StartPos dirct = RigthMid;
	std::stack<StartPos> dirs;
	std::stack<uint> pointsStack;

public:
	MapCountur(mcountor& contur) : contur(contur) {}

	void run(const bool aproxim = false)
	{
		static int poss[16][2] = { {-1, 0}, {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1} };

		dirct = RigthMid;
		while (true)
		{
			char start = (dirct + 6) % 8; // Safe minus 2
			char end = start + 5;
			// Check
			// 1 2 3
			// 0 X 4
			int prevS = getIndex();

			for (; start < end; ++start)
			{
				int* off = poss[(int)start];
				if (tryVal(off[0], off[1]))
				{
					break;
				}
			}

			if (start != end)
			{
				int s = getIndex();
				StartPos old = dirct;
				dirs.push(dirct);
				dirct = (StartPos)(start % 8);

				// Check new disk with the old one
				if (dirct != old || !aproxim)
				{
					contur.push_back(prevS);
					//					contur.push_back(s);
				}

				if (s == stIndex)
				{
					break;
				}
			}
			else
			{
				unset();

				if (pointsStack.size() < 1)
					break;
				//				if (addIndex < 1)
				//					break;

				StartPos old = dirct;
				dirct = dirs.top();
				dirs.pop();

				if (dirct != old || !aproxim)
					contur.pop_back();

				auto p = bc::barvalue::getStatPoint(pointsStack.top());
				pointsStack.pop();
				//contur[addIndex]);
				x = p.x;
				y = p.y;
			}
		}
	}

	void set(const bc::barvalue& p) { points[p.getIndex()] = true; }

	void setStart(int x, int y)
	{
		this->x = x;
		this->y = y;
		stIndex = bc::barvalue::getStatInd(x, y);
	}

private:

	void unset() { points[bc::barvalue::getStatInd(x, y)] = false; }

	bool exists(int xl, int yl)
	{
		auto ds = points.find(bc::barvalue::getStatInd(xl, yl));
		if (ds == points.end())
			return false;

		return ds->second;
	}

	bool tryVal(int oX, int oY)
	{
		if (exists(x + oX, y + oY))
		{
			pointsStack.push(getIndex());
			x += oX;
			y += oY;
			return true;
		}
		return false;
	}

	uint getIndex()
	{
		return bc::barvalue::getStatInd(x, y);
	}
};


void getCountour(const bc::barvector& points, mcountor& contur, bool aproximate)
{
	contur.clear();

	int rect[4]{ 99999999, 99999999, 0, 0 };
	int stY;
	MapCountur dictPoints(contur);
	for (auto& p : points)
	{
		dictPoints.set(p);
		int x = p.getX();
		int y = p.getY();
		if (x < rect[0])
		{
			rect[0] = x;
			stY = y;
		}
		if (x > rect[2])
		{
			rect[2] = x;
		}
		if (y < rect[1])
		{
			rect[1] = y;
		}
		if (y > rect[3])
		{
			rect[3] = y;
		}
	}

	int wid = rect[2] - rect[0];
	int hei = rect[3] - rect[1];

	if (wid < 5 || hei < 5)// || (wid > 1000 && hei > 1000))
		return;

	dictPoints.setStart(rect[0], stY);
	dictPoints.run(aproximate);
}


void saveJson(const string& text, int st)
{
	BackFileWrited file(std::format("D:\\{}.json", st));
	if (file.is_open())
	{
		file << text;
		file.close();
	}
}

export void saveAllJsons(bc::barlinevector* geojson, int num, const BackDirStr& dir)
{
	ProcPoint start;
	int size;

	std::ifstream inputFile("D:\\Programs\\C++\\Barcode\\PrjBarlib\\researching\\coords.txt");
	if (inputFile.is_open())
	{
		BackFileReader inputFile("file.txt");
		BackString line;
		int size;
		int i = 0;

		while (std::getline(inputFile, line))
		{
			if (i == num)
				break;

			size_t commm = line.find("#");
			line = line.substr(0, commm);
			std::vector<std::string> ist;

			for (const auto word : std::views::split(line, "|"))
				ist.push_back(BackString(word.begin(), word.end()));

			if (ist.size() != 3)
				continue;

			start.x = std::stof(ist[0]);
			start.y = std::stof(ist[1]);
			size = std::stoi(ist[2]);

			i++;
		}

		inputFile.close();
	}

	int wid = 500;
	double coof = 0.00001;

	start.x += (wid / 2) * coof;
	start.x -= (wid / 2) * coof;

	saveAsGeojson(geojson[1], dir / "2skat.geojson", start, coof);
	saveAsGeojson(geojson[2], dir / "1skat.geojson", start, coof);
}


void saveAsGeojson(const bc::barlinevector& lines, const BackPathStr& savePath, ProcPoint startOffset, double coof)
{
	BackString json = "{\"type\":\"FeatureCollection\","
		"\"name\":\"Roofs\","
		"\"crs\": { \"type\": \"name\", \"properties\":{\"name\": \"urn:ogc:def:crs:EPSG::3857\" } }," //3857
		"\"features\":[ ";

	json += "";

	for (int i = 0, total = lines.size(); i < total; ++i)
	{
		BackString safsd = "{ \"type\": \"Feature\",";
		safsd += "\"properties\":{\"id\":"; // TODO! Check
		safsd += intToStr(i + 1);
		safsd += "}, \"geometry\": {\"type\":\"MultiPolygon\", \"coordinates\":[[[ ";

		auto& allPoints = lines[i]->getMatrix();
		mcountor cont;
		getCountour(allPoints, cont, true);

		int totalc = cont.size();
		for (int j = 0; j < totalc; ++j)
		{
			const auto& p = bc::barvalue::getStatPoint(cont[j]);
			double x = startOffset.x - p.y * coof;
			double y = startOffset.y + p.x * coof;
			safsd += std::format("[{}, {}],", y, x);
		}

		if (totalc > 0)
		{
			safsd[safsd.length() - 1] = ']';
			safsd += "]]}},";
		}
		else
		{
			safsd += "]]]}},";
		}

		json += safsd;
	}

	json[json.length() - 1] = ']';
	json += "}";

	std::ofstream file(savePath, std::ios::trunc);
	if (file.is_open())
	{
		file << json;
		file.close();
	}
	//	widget->importedMakrers->release();
	//	//	Size2 size = imgsrch.getTileSize();
}


void BarClassifierCache::loadCategories(std::function<void(int classId, const BackString& name)> callback)
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

BarCategories BarClassifierCache::loadCategories()
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

void BarClassifierCache::saveCategories(BarCategories& barclas)
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

void BarClassifierCache::loadImgs(std::function<void(int classId, const BackPathStr& path)> callback, int* categorues, int size)
{
	BackPathStr pathp = "";// Project::getPathSt(BackPath::classfiles);
	CloudBarcodeCreateHelper creator;
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

void BarClassifierCache::save(BarcodeHolder* curBar, int classIndex, BackImage* img)
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
