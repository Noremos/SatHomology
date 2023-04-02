module;
#include <memory>
#include <unordered_set>
#include <algorithm>
#include <functional>

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
import MHashMap;


using LayerMetaProvider = MetadataProvider;


export enum class ReadType
{
	Tiff,
	Simple
};



int getFid(int wid, int s)
{
	return (wid + s - 1) / s;
}

BackImage tiffToImg(ImageReader* reader, const BackPathStr& path, int fctor = 10, bool save = false)
{
	bool rgb = reader->getSamples() > 1;
	int widf = getFid(reader->width(), fctor);
	int heif = getFid(reader->height(), fctor);
	BackImage outr(widf, heif, 3);

	float NAN_VALUE = reader->getNullValue();
	for (int h = 0, hr = 0; h < reader->height(); h += fctor, ++hr)
	{
		const rowptr& rp = reader->getRowData(h);
		for (int w = 0, wr = 0; w < reader->width(); w += fctor, ++wr)
		{
			float value = rp.getFloat(w);
			if (value == NAN_VALUE)
			{
				outr.set(wr, hr, Barscalar(0, 0, 0));
			}
			else if (rgb)
			{
				auto r = rp.getValue(w);
				outr.set(wr, hr, Barscalar(r.rgba.samples[0].s, r.rgba.samples[1].s, r.rgba.samples[2].s));
			}
			else
			{
				if (value < 0)
					value = 0;
				else if (value > 255)
					value = 255;

				outr.set(wr, hr, Barscalar(value, value, value));
			}
		}
	}

	if (save)
		imwrite(path, outr);

	return outr;
}


export class RasterLayer : public IRasterLayer
{
public:
	BackImage mat;

	virtual const LFID getFactoryId() const
	{
		return RASTER_LAYER_FID;
	}

	virtual void saveLoadState(JsonObjectIOState* state, const MetadataProvider& metaFolder)
	{
		IRasterLayer::saveLoadState(state, metaFolder);

		LayerMetaProvider layerMeta(getLayerMeta(metaFolder));
		// int imgId = metaFolder.getUniqueId();
		// state->scInt("mat_id", imgId);
		const BackPathStr path = layerMeta.getSubFolder("mat.png");

		if (state->isReading())
		{
			mat = imread(path);
			// std::remove(path.string().c_str());
		}
		else
			imwrite(path, mat);
	}

	//void release(MetadataProvider mprov)
	//{
	//	const BackPathStr path = mprov.getPath(imgId, ".png");
	//	std::remove(path.string().c_str());
	//}

	void init(const BackImage& src, int mtileSize = DEF_TILE_SIZE)
	{
		mat.assignCopyOf(src);
		prov.init(src.width(), src.height(), src.width(), mtileSize);
	}

	void init(IRasterLayer* layer)
	{
		int wid = layer->displayWidth();
		int hei = layer->displayHeight();

		mat.reinit(wid, hei, 4);
		prov = layer->prov;
	}

	bc::point minmax(const bc::point& p) const
	{
		return { std::min(p.x, mat.width() - 1), std::min(p.y, mat.height() - 1) };
	}


	// ������������ ����� IRasterLayer
	virtual int displayWidth() const override
	{
		return mat.width();
	}
	virtual int displayHeight() const override
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
	id(id), barlineIndex(barlineIndex), start(0), end(0), depth(0), matrSrcSize(0)
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
	std::vector<std::shared_ptr<SimpleLine>> clickResponser;
	int cacheId = -1;

	RasterLineLayer()
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

	BackPathStr getCacheFilePath(const MetadataProvider& metaFolder)
	{
		MetadataProvider m = metaFolder.getSubMeta(getMetaLayerName());
		return m.getSubFolder("cached.bff");
	}

	virtual void release(const MetadataProvider& metaFolder)
	{
		// RasterLayer::release(metaFolder);
		// if (cacheId != -1)
		// {
		// 	std::remove(getCacheFilePath(metaFolder).string().c_str()); // delete file
		// }
	}

	virtual const LFID getFactoryId() const
	{
		return RASTER_LINE_LAYER_FID;
	}

	virtual void saveLoadState(JsonObjectIOState* state, const MetadataProvider& metaFolder)
	{
		RasterLayer::saveLoadState(state, metaFolder);
		state->scInt("cacheId", cacheId);
	}

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

	void init(const BackImage& src, int tileSize = DEF_TILE_SIZE)
	{
		clear();
		mat.assignCopyOf(src);
		clickResponser.resize(mat.length());
		prov.init(src.width(), src.height(), src.width(), tileSize);
	}

	void init(IRasterLayer* layer, const MetadataProvider& metadata)
	{
		int wid = layer->displayWidth();
		int hei = layer->displayHeight();

		clear();
		mat.reinit(wid, hei, 4);
		clickResponser.resize(mat.length());


		prov = layer->prov;

		mkDirIfNotExists(metadata.getSubFolder(getMetaLayerName()));
	}

	void clear()
	{
		clickResponser.clear();
	}

	void setMatrPoint(int x, int y, std::shared_ptr<SimpleLine>& newLine, const Barscalar& color)
	{
		int indLocal = mat.getLineIndex(x, y);
		SimpleLine* existLine = clickResponser[indLocal].get();

		if (existLine == nullptr)
		{
			mat.set(x, y, color);
			clickResponser[indLocal] = newLine;
		}
		else if (existLine == newLine.get()) // Might be due to clickResponser[indLocal2] = newLin
		{
			mat.set(x, y, color);
		}
		else if (existLine->getDeath() < newLine->getDeath())
		{
			// main(depth) < child(depth)
			//newLine->parent = existLine;
			mat.set(x, y, color);
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
	void addSimpleLine(std::shared_ptr<SimpleLine>& line, const bc::barvector& matr, const Barscalar& color, int tileIndex)
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

			setMatrPoint(x, y, line, color);
		}

		getCountourSimple(temp, line->matr);
	}

	bool passLine(const IClassItem* item, const IItemFilter* filter) const
	{
		if (item->getMatrixSize() == 0)
			return false;

		if (filter && !filter->pass(item))
			return false;

		return true;
	}

	void addLine(IdGrater& parentne, int i, const IClassItem* curLine, int tileIndex)
	{
		Barscalar pointCol = RasterLineLayer::colors[rand() % RasterLineLayer::colors.size()];

		std::unordered_set<uint> vals;
		std::shared_ptr<SimpleLine> sl;
		auto curIdKey = curLine->getId();
		auto p = parentne.find(curIdKey);
		if (p != parentne.end())
		{
			sl = p->second;
			sl->barlineIndex = (int)i;
		}
		else
		{
			sl.reset(new SimpleLine(tileIndex, i));
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
			sl->parent.reset(new SimpleLine(tileIndex, -1));
			parentne.insert(std::make_pair(curIdKey, sl->parent));
			//sl->parent->matr = curLine->parent->matr;
		}

		const auto& matr = curLine->getMatrix();
		int depth = curLine->getDeath();
		sl->depth = depth;
		sl->start = curLine->start();
		sl->end = curLine->end();
		sl->matrSrcSize = (int)matr.size();

		addSimpleLine(sl, matr, pointCol, tileIndex);
	}

	void addHolder(const IClassItemHolder& items, int tileIndex, const IItemFilter* filter)
	{
		IdGrater parentne;

		const auto& vec = items.getItems();
		for (size_t i = 0; i < vec.size(); ++i)
		{
			auto curLine = vec.at(i);
			if (!passLine(curLine, filter))
				continue;

			addLine(parentne, i, curLine, tileIndex);
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

	// notSave
	// MetadataProvider* mprov;

	int subImageIndex = 0;
	std::vector<BackImage> images;
	int subImgSize;

	~RasterFromDiskLayer()
	{
		closeImages();
	}

	virtual const LFID getFactoryId() const
	{
		return RASTER_DISK_LAYER_FID;
	}

	virtual void saveLoadState(JsonObjectIOState* state, const MetadataProvider& metaFolder)
	{
		// this->mprov = &metaFolder;
		IRasterLayer::saveLoadState(state, metaFolder);

		state->scPath("imgPath", imgPath);
		state->scInt("subImgSize", subImgSize);
		state->scInt("subImageIndex", subImageIndex);
		int iimgType = (int)imgType;
		state->scInt("imgType", iimgType);
		imgType = (ReadType)iimgType;

		if (state->isReading())
		{
			openReader();
			readImagesFromCache(metaFolder);
			setSubImage(subImageIndex);
		}
	}

	void open(const BackPathStr& path, MetadataProvider metaPath)
	{
		closeReader();

		imgPath = path;
		name = imgPath.filename().string();
		// this->mprov = &metaPath;

		openReader();
		if (!reader->ready)
			return;

		writeImages(metaPath);
		setSubImage(0);
	}


	int getSubImage()
	{
		return subImageIndex;
	}

	void setSubImage(int imgIndex)
	{
		int displayWid = images[imgIndex].width();
		if (imgType == ReadType::Tiff)
		{
			TiffReader* treader = static_cast<TiffReader*>(reader);
			treader->setCurrentSubImage(imgIndex);
			const auto& tags = treader->getTags();

			auto& p = tags.ModelTiepointTag.points[0];
			cs.setOrigin(p.X, p.Y);
			cs.setScale(tags.ModelPixelScaleTag.x, tags.ModelPixelScaleTag.y);
			// cs.init(tags.) // TODO: fixme

			subImageIndex = imgIndex;
		}
		else
		{
			subImageIndex = 0;
		}
		prov.update(reader->width(), reader->height(), displayWid);
		if (prov.tileSize + tileOffset > prov.width)
		{
			tileOffset = prov.width - prov.tileSize;
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
			//info.push_back({ realWidth(), realHeight() });
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

	void writeImages(MetadataProvider metaprov)
	{
		if (!reader)
			return;

		MetadataProvider layerMeta(getLayerMeta(metaprov));
		BackDirStr tiles = layerMeta.getSubFolder("tiles");

		closeImages();
		images.clear();

		layerMeta.mkdir();

		mkDirIfNotExists(tiles);

		if (imgType == ReadType::Tiff)
		{
			TiffReader* trear = dynamic_cast<TiffReader*>(reader);
			subImgSize = trear->getSubImageSize();
			for (int i = 0, k = 0; i < subImgSize; ++i)
			{
				//int factor = 1;
				trear->setCurrentSubImage(i);
				if (reader->width() <= 2000)
					images.push_back(tiffToImg(reader, tiles / (intToStr(k++) + ".png"), 1.0, true));
			}

			if (images.size() == 0)
			{
				int factor = reader->width() / 2000;
				images.push_back(tiffToImg(reader, tiles / (intToStr(0) + ".png"), factor, true));
			}

			trear->setCurrentSubImage(subImageIndex);
		}
		else
		{
			images.push_back(tiffToImg(reader, tiles / (intToStr(0) + ".png"), 1, true));
		}

		subImgSize = images.size();
	}


	void readImagesFromCache(const MetadataProvider& metaprov)
	{
		if (!reader)
			return;

		LayerMetaProvider layerMeta(getLayerMeta(metaprov));
		BackDirStr tiles = layerMeta.getSubFolder("tiles");

		for (int i = 0; i < subImgSize; ++i)
		{
			// int factor = 1;
			BackPathStr stp = tiles / (intToStr(i) + ".png");
			images.push_back(imread(stp));
		}
	}


	void closeImages()
	{
		//for (int i = 0; i < images.size(); ++i)
		//	delete images[i];
		images.clear();
	}

	// ������������ ����� IRasterLayer
	virtual int displayWidth() const override
	{
		return reader->width() * prov.displayFactor;
	}
	virtual int displayHeight() const override
	{
		return reader->height() * prov.displayFactor;
	}
	virtual int realWidth() const override
	{
		return reader->width();
	}
	virtual int realHeight() const override
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
		return images[0];
	}
	virtual const BackImage* getCachedImage() const override
	{
		return &images[0];
	}
};
