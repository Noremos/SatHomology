module;
#include <memory>
#include <unordered_set>
#include <algorithm>

#include "../../Bind/Common.h"

export module RasterLayers;
//import std.core;


import BarholdersModule;
import IOCore;
import Platform;
import CacheFilesModule;
import GeoprocessorModule;

import LayersCore;
import JsonCore;
import ClassifierInterface;
import MetadataIOCore;


export class RasterLayer : public ILayer
{
public:
	BackImage mat;

	RasterLayer(float displayFactor = 1, int tileSize = 0, int width = 0) : ILayer(displayFactor, tileSize, width)
	{ }

	bc::point minmax(const bc::point& p) const
	{
		return { std::min(p.x, mat.width() - 1), std::min(p.y, mat.height() - 1) };
	}

	virtual void saveLoadState(JsonObjectIOState* state, const BackDirStr& metaFolder)
	{
		//int id = -1;

		//BackString name;
		//int iconId = -1;
		//LayerProvider prov;
		////ILayer::writeJson(json, metaFolder, counter);
		//++counter;
		//BackPathStr fulp = metaFolder / intToStr(counter) / ".png";
		//imwrite(fulp, mat);
		// json["matId"] = counter;

				//ILayer::readJson(json, metaFolder);
		// int counter = json["matId"].get<int>();
		// BackPathStr fulp = metaFolder / intToStr(counter) / ".png";
		// mat = imread(fulp);
				//ILayer::writeJson(json, metaFolder, counter);
		//++counter;
		//BackPathStr fulp = metaFolder / intToStr(counter) / ".png";
		//imwrite(fulp, mat);
		// json["matId"] = counter;
	}
};


export class DrawPrimetive
{
public:

	enum class Type
	{
		points,
		polygon,
		multipolygon
	};

	Type type;
	Barscalar color;
	std::vector<bc::point> draws;
};

export class PrimetiveLayer : public ILayer
{
public:
	std::vector<DrawPrimetive> primetives;
};


export struct SimpleLine
{
	int id, barlineIndex;
	SimpleLine(int id = 0, int barlineIndex = 0) :
	id(id), barlineIndex(barlineIndex), start(0), end(0), depth(0), matrSrcSize(0), parent(nullptr)
	{}
	//	ushort counter = 0;
	Barscalar start, end;

	int depth;
	int matrSrcSize;

	std::shared_ptr<SimpleLine> parent;

	bc::barvector matr;

	int getDeath()
	{
		return depth;
	}
};

export using IdGrater = MMMAP<size_t, std::shared_ptr<SimpleLine>>;

export class RasterLineLayer : public RasterLayer
{
public:
	static std::vector<Barscalar> colors;

	RasterLineLayer(float displayFactor = 1, int tileSize = 0, int width = 0) : RasterLayer(displayFactor, tileSize, width)
	{
		if (colors.size() == 0)
		{
			srand(300);
			colors.push_back(Barscalar(255, 0, 0));
			colors.push_back(Barscalar(0, 0, 0));
			colors.push_back(Barscalar(0, 255, 0));
			colors.push_back(Barscalar(0, 0, 255));
			for (int k = 0; k < 40; ++k)
				colors.push_back(Barscalar(5 + rand() % 250, 5 + rand() % 250, 5 + rand() % 250));

			colors.push_back(Barscalar(255, 255, 255));
		}
	}
	std::vector<std::shared_ptr<SimpleLine>> clickResponser;

	//virtual void readJson(const BackJson& json, const BackDirStr& metaFolder)
	//{
	//	RasterLayer::readJson(json, metaFolder);
	//}

	//virtual void writeJson(BackJson& json, const BackDirStr& metaFolder, int& counter)
	//{
	//	RasterLayer::writeJson(json, metaFolder, counter);

	//	for (int i=0;i < clickResponser.size(); i++)
	//	{
	//		// id, id in bar
	//	}
	//}

	void init(const BackImage& src)
	{
		clear();
		mat.assignCopyOf(src);
		clickResponser.resize(mat.length());
	}

	void init(int wid, int hei)
	{
		clear();
		mat.reinit(wid, hei, 4);
		clickResponser.resize(mat.length());
	}

	void clear()
	{
		clickResponser.clear();
	}

	void setMatrPoint(int x, int y, std::shared_ptr<SimpleLine>& newLine)
	{
		int indLocal = mat.getLineIndex(x, y);
		SimpleLine* existLine = clickResponser[indLocal].get();

		if (existLine == nullptr)
		{
			clickResponser[indLocal] = newLine;
		}
		else if (existLine->getDeath() < newLine->getDeath())
		{
			// main(depth) < child(depth)
			//newLine->parent = existLine;
			clickResponser[indLocal] = newLine;

			int ylek = 2;
			for (int i = MAX(x - ylek, 0); i < std::min(x + ylek, mat.wid()); i++)
			{
				for (int j = MAX(y - ylek, 0); j < std::min(y + ylek, mat.hei()); j++)
				{
					int indLocal2 = mat.getLineIndex(i, j);

					clickResponser[indLocal2] = newLine;
				}
			}
		}
	}

	using ColorGrater = std::function<Barscalar(const IClassItem* item, bool& bad)>;
	void addSimpleLine(std::shared_ptr<SimpleLine> line, const bc::barvector& matr, Barscalar color, int tileIndex)
	{
		auto tileProv = prov.tileByIndex(tileIndex);

		std::unordered_set<uint> vals;
		bc::barvector temp;
		for (const auto& pm : matr)
		{
			auto o = tileProv.toGlobal(pm.getX(), pm.getY());
			int x = (std::min)(mat.wid() - 1, o.x);
			int y = (std::min)(mat.hei() - 1, o.y);
			uint index = bc::barvalue::getStatInd(x, y);
			if (vals.find(index) != vals.end())
				continue;

			vals.insert(index);

			bc::point cp = bc::barvalue::getStatPoint(index);
			temp.push_back(bc::barvalue(cp, pm.value));

			mat.set(x, y, color);
			setMatrPoint(x, y, line);
		}

		getCountourSimple(temp, line->matr);
	}

	void addLine(int i, IdGrater& parentne, const IClassItem* curLine, int tileIndex, const FilterInfo* filter, ColorGrater grater)
	{
		const auto& matr = curLine->getMatrix();
		if (matr.size() == 0)
			return;

		if (filter && !curLine->passFilter(*filter))
			return;

		Barscalar pointCol(255, 0, 0);
		if (grater)
		{
			bool show = true;
			pointCol = grater(curLine, show);
			if (!show)
				return;
		}
		else
			pointCol = RasterLineLayer::colors[rand() % RasterLineLayer::colors.size()];

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

		curIdKey = curLine->getParentId();
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
		sl->start = curLine->start();
		sl->end = curLine->end();
		sl->matrSrcSize = (int)matr.size();

		addSimpleLine(sl, matr, pointCol, tileIndex);
	}

	void addHolder(const IClassItemHolder& items, int tileIndex, const FilterInfo* filter, ColorGrater grater)
	{
		auto tileProv = prov.tileByIndex(tileIndex);
		IdGrater parentne;

		const auto& vec = items.getItems();
		for (size_t i = 0; i < vec.size(); ++i)
		{
			addLine(i, parentne, vec.at(i), tileIndex, filter, grater);
		}
	}
};

std::vector<Barscalar> RasterLineLayer::colors;
