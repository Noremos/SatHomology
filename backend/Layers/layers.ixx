module;
#include <memory>
#include <unordered_set>
#include <algorithm>
#include <functional>
#include <cassert>

#include "../../Bind/Common.h"

export module RasterLayers;
import LayersCore;

import BarcodeModule;
import IOCore;
import Platform;
import GeoprocessorModule;
import ItemInterface;

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
	const int rwid = reader->width();
	const int rhei = reader->height();
	const bool rgb = reader->getSamples() > 1;
	const float NAN_VALUE = reader->getNullValue();

	if (fctor == 1)
	{
		BackImage out(rwid, rhei, 3);

		auto rp = reader->getRect(0,0, rwid, rhei);

		if (rgb)
		{
			for (size_t i = 0; i < out.length(); i++)
			{
				auto value = rp.data.getValueV(i);
				if (value.avg() == NAN_VALUE)
				{
					out.setLiner(i, Barscalar(0, 0, 0));
				}
				else
				{
					out.setLiner(i, Barscalar(0, 0, 0));

					auto& r = value.val.rgba;
					out.setLiner(i, Barscalar(r.samples[0].s, r.samples[1].s, r.samples[2].s));
				}
			}
		}
		else
		{
			for (size_t i = 0; i < out.length(); i++)
			{
				float value = rp.data.getFloat(i);
				if (value == NAN_VALUE)
				{
					out.setLiner(i, Barscalar(0, 0, 0));
				}
				else
				{
					if (value < 0)
						value = 0;
					else if (value > 255)
						value = 255;

					out.setLiner(i, Barscalar(value, value, value));
				}
			}
		}

		if (save)
			imwrite(path, out);
		return out;
	}
	else
	{
		const int widf = getFid(rwid, fctor);
		const int heif = getFid(reader->height(), fctor);
		BackImage outr(widf, heif, 3);

		float NAN_VALUE = reader->getNullValue();
		for (int h = 0, hr = 0; h < reader->height(); h += fctor, ++hr)
		{
			const rowptr& rp = reader->getRowData(h);
			if (rgb)
			{
				for (int w = 0, wr = 0; w < rwid; w += fctor, ++wr)
				{
					auto value = rp.getValueV(w);
					if (value.avg() == NAN_VALUE)
					{
						outr.set(wr, hr, Barscalar(0, 0, 0));
					}
					else
					{
						auto& r = value.val.rgba;
						outr.set(wr, hr, Barscalar(r.samples[0].s, r.samples[1].s, r.samples[2].s));
					}
				}
			}
			else
			{
				for (int w = 0, wr = 0; w < rwid; w += fctor, ++wr)
				{
					float value = rp.getFloat(w);
					if (value == NAN_VALUE)
					{
						outr.set(wr, hr, Barscalar(0, 0, 0));
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
		}

		if (save)
			imwrite(path, outr);
		return outr;
	}
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
		layerMeta.mkdir();
		// int imgId = metaFolder.getUniqueId();
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

	void init(int wid, int hei, int chls, int mtileSize = DEF_TILE_SIZE)
	{
		mat.reinit(wid, hei, chls);
		prov.init(wid, hei, wid, mtileSize);
	}

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
	virtual void setCache(size_t)
	{ }

	virtual int realWidth() const override
	{
		return mat.width();
	}
	virtual int realHeight() const override
	{
		return mat.height();
	}

	virtual float displayWidth() const override
	{
		return mat.width();
	}
	virtual float displayHeight() const override
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

	virtual SubImgInf getSubImgInf()
	{
		return SubImgInf(mat.width(), mat.height());
	}

	void setSubImage(int,bool = false) {}
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
	int parentlayerId = -1;

	// Sub to display factor
	float subToRealFactor = 1.f;

	RasterLineLayer()
	{
		if (colors.size() == 0)
		{
			// srand(300);
			colors.push_back(Barscalar(255, 0, 0));
			colors.push_back(Barscalar(0, 0, 0));
			colors.push_back(Barscalar(0, 255, 0));
			colors.push_back(Barscalar(0, 0, 255));
			for (int k = 0; k < 40; ++k)
				colors.push_back(Barscalar(5 + rand() % 250, 5 + rand() % 250, 5 + rand() % 250));

			colors.push_back(Barscalar(255, 255, 255));
		}
	}

	static const Barscalar& getRandColor(size_t id)
	{
		return colors[id % colors.size()];
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
		state->scInt("parentLayerId", parentlayerId);
		state->scFloat("subToRealFactor", subToRealFactor); // TODO: rename it to SubToDsiplay
	}

	virtual int realWidth() const override
	{
		return mat.width() * subToRealFactor; // Invalid
	}
	virtual int realHeight() const override
	{
		return mat.height() * subToRealFactor; // Invaild;
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

	//void init(const BackImage& src, int tileSize = DEF_TILE_SIZE)
	//{
	//	clickResponser.clear();
	//	mat.assignCopyOf(src);
	//	clickResponser.resize(mat.length());
	//	printf("alloced for preview: %d\n", clickResponser.size());
	//	prov.init(src.width(), src.height(), src.width(), tileSize);
	//}

	void init(IRasterLayer* layer, const MetadataProvider& metadata)
	{
		assert(layer != this);

		subToRealFactor = layer->realWidth() / layer->displayWidth(); // SubToDsiplay
		int wid = layer->displayWidth();
		int hei = layer->displayHeight();

		resetResponser(wid, hei);
		parentlayerId = layer->id;
		prov = layer->prov;

		mkDirIfNotExists(metadata.getSubFolder(getMetaLayerName()));
	}

	void clearResponser()
	{
		int wid = mat.width();
		int hei = mat.height();
		assert(mat.length() > 1);
		resetResponser(wid, hei);
	}

	void resetResponser(int wid, int hei)
	{
		clickResponser.clear();
		mat.reinit(wid, hei, 4);
		clickResponser.resize(mat.length());
		printf("alloced for preview: %d\n", clickResponser.size());
	}


	std::mutex addToMapMutex;
	void setMatrPoint(int x, int y, std::shared_ptr<SimpleLine>& newLine, const Barscalar& color)
	{
		std::lock_guard<std::mutex> guard(addToMapMutex);

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
			mat.set(x, y, color);
			clickResponser[indLocal] = newLine;
		}
	}

	// using ColorGrater = std::function<Barscalar(const IClassItem* item, bool& bad)>;

	bool passLine(const IClassItem* item, const IItemFilter* filter) const
	{
		if (item->getMatrixSize() == 0)
			return false;

		if (filter && !filter->pass(item))
			return false;

		return true;
	}

	void addLine(IdGrater& parentne, int i, const IClassItem* curLine, const TileProvider& tileProv)
	{
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
			sl.reset(new SimpleLine(tileProv.index, i));
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
			sl->parent.reset(new SimpleLine(tileProv.index, -1));
			parentne.insert(std::make_pair(curIdKey, sl->parent));
			//sl->parent->matr = curLine->parent->matr;
		}

		const auto& matr = curLine->getMatrix();
		int depth = curLine->getDeath();
		sl->depth = depth;
		sl->start = curLine->start();
		sl->end = curLine->end();
		sl->matrSrcSize = (int)matr.size();

		// Add line
		Barscalar pointCol = RasterLineLayer::colors[rand() % RasterLineLayer::colors.size()];

		// Countur
		DictWrap dictPoints;
		for (auto& p : matr)
		{
			dictPoints.set(p.getIndex());
			int x = p.getX();
			int y = p.getY();
			if (x > dictPoints.hei)
			{
				dictPoints.hei = x;
			}
			if (y > dictPoints.wid)
			{
				dictPoints.wid = y;
			}
		}

		auto& outMatr = sl->matr;
		for (const auto& pm : matr)
		{
			// We have:
			// - a real img size (input)
			// - a sub img size (selected subimg in input)
			// - a display img (for LineRaster).

			// Get a pixels form a sub
			int x = pm.getX();
			int y = pm.getY();


			// Cast sub point to display (mat variable) via tileProv
			BackPixelPoint op = tileProv.tileToPreview(x, y);
			op.x = (std::min)(mat.wid() - 1, op.x);
			op.y = (std::min)(mat.hei() - 1, op.y);

			// Set display point
			setMatrPoint(op.x, op.y, sl, pointCol);

			// Get countur
			if (dictPoints.hasCorners(x, y)) // Skip
			{
				continue;
			}

			// Cast to a real img via factor and save for draw
			outMatr.push_back(bc::barvalue(op.x * subToRealFactor, op.y * subToRealFactor, pm.value));
		}
	}

	void addHolder(const IClassItemHolder& items, const TileProvider& tileProv, const IItemFilter* filter)
	{
		IdGrater parentne;

		for (size_t i = 0; i < items.getItemsCount(); ++i)
		{
			auto curLine = items.getItem(i);
			if (!passLine(curLine, filter))
				continue;

			addLine(parentne, i, curLine, tileProv);
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
	BackPixelPoint realSize;

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

		setSubImage(0, true);
		if (!reader->ready)
			return;
	}

	void cache(const MetadataProvider& metaPath)
	{
		writeImages(metaPath);
	}

	int getSubImage()
	{
		return subImageIndex;
	}

	SubImgInf getSubImgInf()
	{
		return SubImgInf(reader->width(), reader->height());
	}

	void setSubImage(int imgIndex, bool overrideCs = false)
	{
		int iwid = 2000;

		if (imgType == ReadType::Tiff)
		{
			TiffReader* treader = static_cast<TiffReader*>(reader);
			for (int i = imgIndex; i < treader->getSubImageSize(); ++i)
			{
				//int factor = 1;
				int tegWid = treader->getSubImg(i)->tags.ImageWidth;
				if (tegWid <= 2000)
				{
					iwid = tegWid;
					break;
				}
			}

			treader->setCurrentSubImage(imgIndex);

			if (overrideCs)
			{
				const auto& tags = treader->getTags();

				auto& p = tags.ModelTiepointTag.points[0];
				cs.setOrigin(p.X, p.Y);
				cs.setScale(tags.ModelPixelScaleTag.x, tags.ModelPixelScaleTag.y);
				cs.proj.reinit(treader->getProjection());
			}

			subImageIndex = imgIndex;
		}
		else
		{
			iwid = reader->width();
			subImageIndex = 0;
		}

		prov.update(realSize.x, realSize.y, iwid); // restor it when images will be dropped

		// prov.update(realWidth, realHeight, reader->width()); // restor it when images will be dropped
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

		TiffReader* treader = static_cast<TiffReader*>(reader);

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

		if (imgType == ReadType::Tiff)
		{
			TiffReader* treader = static_cast<TiffReader*>(reader);
			auto& realTags = treader->getSubImg(0)->tags;
			realSize.x = realTags.ImageWidth;
			realSize.y = realTags.ImageLength;
		}
		else
		{
			realSize.x = reader->width();
			realSize.y = reader->height();
		}
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
			TiffReader* trear = static_cast<TiffReader*>(reader);
			subImgSize = trear->getSubImageSize();
			for (int i = 0, k = 0; i < subImgSize; ++i)
			{
				//int factor = 1;
				trear->setCurrentSubImage(i);
				if (reader->width() <= 2000)
				{
					images.push_back(tiffToImg(reader, tiles / (intToStr(k++) + ".png"), 1.0, true));
					break;
				}
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

	virtual void setCache(size_t rowsSize)
	{
		if (imgType == ReadType::Tiff)
		{
			TiffReader* trear = static_cast<TiffReader*>(reader);
			trear->cachedRows.setMaxSize(rowsSize);
			trear->setMaxTilesCacheSize(rowsSize);
		}
	}

	virtual float displayWidth() const override
	{
		return realSize.x / prov.displayFactor;
	}
	virtual float displayHeight() const override
	{
		return realSize.y / prov.displayFactor;
	}
	virtual int realWidth() const override
	{
		return realSize.x;
	}
	virtual int realHeight() const override
	{
		return realSize.y;
	}
	virtual BackImage getRect(int stX, int stRow, int wid, int hei) override
	{
		DataRect r = reader->getRect(stX, stRow, wid, hei);
		return BackImage(r.wid, r.hei, r.samples(), r.getData());
	}
	virtual BackImage getImage(const int) const override
	{
		return images[0];
	}
	virtual const BackImage* getCachedImage() const override
	{
		return &images[0];
	}
};
