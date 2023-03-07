module;
#include <memory>
#include <random>
#include <iostream>
#include <functional>

#include "../../Bind/Common.h"

export module Classifiers;
//import std.core;


import BarholdersModule;
import IOCore;
import Platform;
import CacheFilesModule;

import LayersCore;
import BarcodeModule;
import JsonCore;

import TrainIO;


export class barclassificator
{
public:
	struct ClassData
	{
		int classId;
		bc::Barcontainer container;
		MMMAP<size_t, int> cacheIndex;
	};
	std::vector<ClassData> classes;
	BackPathStr dbPath;

	static BackString className()
	{
		return "CLASSIC";
	}

	void loadClasses(const BarCategories& categs, const BackPathStr& path)
	{
		dbPath = path;
		classes.clear();
		for (auto categ : categs.categs)
		{
			addClass(categ.id);
		}
	}

	ClassData& getClass(int id)
	{
		for (int i = 0; i < classes.size(); i++)
		{
			if (classes[i].classId == id)
			{
				return classes[i];
			}
		}

		throw;
	}

	void addClass(int id)
	{
		ClassData nd;
		nd.classId = id;
		classes.push_back(nd);
	}

	void removeClass(int id)
	{
		for (int i = 0; i < classes.size(); i++)
		{
			if (classes[i].classId == id)
			{
				classes.erase(classes.begin() + i);
				break;
			}
		}
	}

	size_t addData(int classInd, bc::barline* raw, BackImage* icon, bool extractLine = false)
	{
		bc::Baritem* item = new bc::Baritem();
		if (extractLine)
			raw->extractChilred(item->barlines, true, false);
		else
			raw->getChilredAsList(item->barlines, true, true, false);

		item->relen();
		item->setType();

		assert(classes.size() !=0);
		auto classHolder = getClass(classInd);

		std::ostringstream st;
		GeoBarRasterCache cached;
		cached.openWrite(st);
		cached.save(item, 0);
		cached.close();

		ClassDataIO io;
		io.openWrite(dbPath);
		vbuffer temp;
		temp.setData(reinterpret_cast<uchar*>(st.str().data()), st.str().length(), false);
		size_t id = io.save(classInd, temp, icon);

		classHolder.cacheIndex.insert(std::make_pair(id, classHolder.container.count()));
		classHolder.container.addItem(item);

		return id;
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

	void removeData(int classId, size_t id)
	{
		auto classHolder = getClass(classId);
		auto it = classHolder.cacheIndex.find(id);
		if (it != classHolder.cacheIndex.end())
		{
			delete classHolder.container.exractItem(it->second);
			classHolder.cacheIndex.erase(it);
			ClassDataIO io;
			io.openWrite(dbPath);
			io.remove(id);
		}
	}

	int getType(bc::barline* raw)
	{
		bc::Baritem newOne;
		raw->getChilredAsList(newOne.barlines, true, false, false);
		newOne.relen();

		auto cp = bc::CompireStrategy::CommonToLen;
		float res = 0;

		int maxInd = -1;
		float maxP = res;
		for (size_t i = 0; i < classes.size(); i++)
		{
			float ps = classes[i].container.compireBest(&newOne, cp);
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
		return Barscalar(arr[0].asDouble(), arr[1].asDouble(), arr[2].asDouble());
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