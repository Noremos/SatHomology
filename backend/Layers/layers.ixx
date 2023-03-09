module;
#include <memory>
#include <unordered_set>
#include <algorithm>

#include "../../Bind/Common.h"

export module RasterLayers;
//import std.core;
import LayersCore;

import BarcodeModule;
import IOCore;
import Platform;
import GeoprocessorModule;
import ClassifierInterface;

import MetadataIOCore;

import ImgReader;
import SimpleImgReaderModule;



export enum class ReadType
{
	Tiff,
	Simple
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


int getFid(int wid, int s)
{
	return (wid + s - 1) / s;
}

BackImage* tiffToImg(ImageReader* reader, const BackPathStr& path, int fctor = 10, bool save = false)
{
	bool rgb = reader->getSamples() > 1;
	int widf = getFid(reader->width(), fctor);
	int heif = getFid(reader->height(), fctor);
	MatrImg* outr = new MatrImg(widf, heif, 3);

	float NAN_VALUE = reader->getNullValue();
	for (int h = 0, hr = 0; h < reader->height(); h += fctor, ++hr)
	{
		const rowptr& rp = reader->getRowData(h);
		for (int w = 0, wr = 0; w < reader->width(); w += fctor, ++wr)
		{
			float value = rp.getFloat(w);
			if (value == NAN_VALUE)
			{
				outr->set(wr, hr, Barscalar(0, 0, 0));
			}
			else if (rgb)
			{
				auto r = rp.getValue(w);
				outr->set(wr, hr, Barscalar(r.rgba.samples[0].s, r.rgba.samples[1].s, r.rgba.samples[2].s));
			}
			else
			{
				if (value < 0)
					value = 0;
				else if (value > 255)
					value = 255;

				outr->set(wr, hr, Barscalar(value, value, value));
			}
		}
	}

	if (save)
		imwrite(path, *outr);
	return outr;
}


export class RasterLayer : public IRasterLayer
{
public:
	BackImage mat;

	RasterLayer(float displayFactor = 1, int tileSize = 0, int width = 0) : IRasterLayer(displayFactor, tileSize, width)
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

	// ������������ ����� IRasterLayer
	virtual int displayWidth() const override
	{
		return mat.width();
	}
	virtual int displayHeigth() const override
	{
		return mat.height();
	}
	virtual int realWidth() const override
	{
		return mat.width();
	}
	virtual int realHeigth() const override
	{
		return mat.height();
	}

	virtual BackImage getRect(int stX, int stRow, int wid, int hei)
	{
		return mat.getRect(stX, stRow, wid, hei);
	}
	virtual BackImage getImage(const int max) const override
	{
		return mat;
	}
	virtual const BackImage* getCachedImage() const override
	{
		return &mat;
	}

	void setSubImage(int) {}
	int getSubImage() {return 0;}
	int getFirstSmallIndex(const int maxSize = 2000) {return 0;}
	std::vector<SubImgInf> getSubImageInfos()
	{
		std::vector<SubImgInf> sub;
		//sub.push_back({ mat.width(), mat.height()});
		return sub;
	}
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


export class RasterFromDiskLayer : public IRasterLayer
{
public:
	ImageReader* reader = nullptr;

	ReadType imgType;
	BackPathStr imgPath;

	// nnotSave
	BackDirStr metaPath;

	int subImageIndex = 0;
	std::vector<BackImage*> images;

	RasterFromDiskLayer(float displayFactor = 1, int tileSize = 0, int width = 0) : IRasterLayer(displayFactor, tileSize, width)
	{ }

	void open(const BackPathStr& path, const BackDirStr& metaPath)
	{
		closeReader();

		imgPath = path;
		this->metaPath = metaPath;

		openReader();
		if (!reader->ready)
			return;
	}


	int getSubImage()
	{
		return subImageIndex;
	}

	void setSubImage(int imgIndex)
	{
		int displayWid = images[imgIndex]->width();
		if (imgType == ReadType::Tiff)
		{
			dynamic_cast<TiffReader*>(reader)->setCurrentSubImage(imgIndex);
			prov.displayFactor = (float)reader->width() / displayWid;
			subImageIndex = imgIndex;
		}
		else
		{
			subImageIndex = 0;
			prov.displayFactor = 1.0f;
		}
	}

	int getFirstSmallIndex(const int maxSize = 2000)
	{
		if (!reader)
			return -1;
		if (imgType != ReadType::Tiff)
			return 0;

		TiffReader* treader = dynamic_cast<TiffReader*>(reader);

		int s = treader->getSubImageSize();
		//BackPathStr tiles = getPath(BackPath::tiles);
		for (int i = 0; i < s; ++i)
		{
			treader->setCurrentSubImage(i);
			if (treader->width() <= maxSize)
			{
				return i;
			}
		}

		return -1;
	}

	std::vector<SubImgInf> getSubImageInfos()
	{
		std::vector<SubImgInf> info;
		if (imgType != ReadType::Tiff)
		{
			//info.push_back({ realWidth(), realHeigth() });
			return info;
		}

		TiffReader* treader = dynamic_cast<TiffReader*>(reader);

		auto osd = treader->getSumImageInfos();
		for (auto& o : osd)
		{
			info.push_back({ o.width, o.height });
		}
		return info;
	}


	void closeReader()
	{
		if (reader)
		{
			reader->close();
			delete reader;
			reader = nullptr;
		}
	}

	void openReader()
	{
		if (!reader)
		{
			const auto& ext = imgPath.extension();
			if (ext == ".tiff" || ext == ".tif")
			{
				imgType = ReadType::Tiff;
				reader = new TiffReader();
			}
			else
			{
				imgType = ReadType::Simple;
				reader = new SimpleImgReader();
			}
		}

		if (!reader->ready)
			reader->open(imgPath.string());
	}


	void writeImages(MetadataProvider& mprov)
	{
		if (!reader)
			return;

		images.clear();

		BackDirStr tiles = mprov.getSubFolter("tiles");
		if (imgType == ReadType::Tiff)
		{
			TiffReader* trear = dynamic_cast<TiffReader*>(reader);
			int s = trear->getSubImageSize();
			for (int i = 0; i < s; ++i)
			{
				int factor = 1;
				trear->setCurrentSubImage(i);
				if (reader->width() > 2000)
				{
					images.push_back(nullptr);
				}
				else
					images.push_back(tiffToImg(reader, tiles / (intToStr(i) + ".png"), factor, true));
			}

			if (images.size() == 0)
				images.push_back(tiffToImg(reader, tiles / (intToStr(s) + ".png"), 10, true));
		}
		else
		{
			images.push_back(tiffToImg(reader, tiles / (intToStr(0) + ".png"), 1, true));
		}
	}


	void readImages(MetadataProvider& mprov)
	{
		if (!reader)
			return;

		BackDirStr tiles = mprov.getSubFolter("tiles");
		if (imgType == ReadType::Tiff)
		{
			TiffReader* treader = dynamic_cast<TiffReader*>(reader);
			int s = treader->getSubImageSize();
			closeImages();
			for (int i = 0; i < s; ++i)
			{
				treader->setCurrentSubImage(i);
				if (reader->width() > 2000)
				{
					images.push_back(nullptr);
				}
				else
					images.push_back(tiffToImg(reader, tiles / (intToStr(i) + ".png"), 1, false));
			}

			if (images.size() == 0)
				images.push_back(tiffToImg(reader, tiles / (intToStr(s) + ".png"), 10, false));
		}
		else
			images.push_back(tiffToImg(reader, tiles / (intToStr(0) + ".png"), 1, false));
	}


	void closeImages()
	{
		for (int i = 0; i < images.size(); ++i)
			delete images[i];

		images.clear();
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

	// ������������ ����� IRasterLayer
	virtual int displayWidth() const override
	{
		return reader->width() * prov.displayFactor;
	}
	virtual int displayHeigth() const override
	{
		return reader->height() * prov.displayFactor;
	}
	virtual int realWidth() const override
	{
		return reader->width();
	}
	virtual int realHeigth() const override
	{
		return reader->height();
	}
	virtual BackImage getRect(int stX, int stRow, int wid, int hei) override
	{
		DataRect r = reader->getRect(stX, stRow, wid, hei);
		return BackImage(r.wid, r.hei, r.samples(), r.getData());
	}
	virtual BackImage getImage(const int max) const override
	{
		return *images[0];
	}
	virtual const BackImage* getCachedImage() const override
	{
		return images[0];
	}
};
