module;

#include <unordered_set>
#include <random>
#include <memory>

#include "../side/PortFileDialog.h"
#include "../side/sago/platform_folders.h"
#include "../side/flat_hash_map/unordered_map.hpp"

#include "../frontend/GuiCommon.h"
#include "../frontend/GuiWidgets.h"

export module ForntnedModule;

import ProjectModule;
import ImgReader;
import GeoprocessorModule;
import Platform;
import IOCore;
import BarcodeModule;
import LayersCore;
import ClassifiersCore;

// Todo.
// 2 режима
export class GuiBackend
{
	enum class GuiState
	{
		Empty = 0,
		ImageLoaded,
		BarcodeCreated
	};
public:
	GuiBackend();
	~GuiBackend()
	{
		clear();
		Project::dropProject();
	}

	bool isImageLoaded() const
	{
		return state >= GuiState::ImageLoaded;
	}

	bool isBarcodeCreated() const
	{
		return state >= GuiState::BarcodeCreated;
	}

	void clear();

	struct ComFinder
	{
		double totalMtrixCountMin = 1.f; // Кол-во в процентах
		double totalMtrixCountMax = 0.f; // Кол-во в процентах
		int minDepth = 20;
		int maxDepth = 0;
		Barscalar minStart = Barscalar(255, 255, 255); // Non-random
		Barscalar maxStart = Barscalar(0, 0, 0); // Non-random
		Barscalar minLen = Barscalar(255, 255, 255); // Non-random
		Barscalar maxLen = Barscalar(0, 0, 0); // Non-random

		void clear()
		{
			totalMtrixCountMin = 1.f; // Кол-во в процентах
			totalMtrixCountMax = 0.f; // Кол-во в процентах
			minDepth = 20;
			maxDepth = 0;
			minStart = Barscalar(255, 255, 255); // Non-random
			maxStart = Barscalar(0, 0, 0); // Non-random
			minLen = Barscalar(255, 255, 255); // Non-random
			maxLen = Barscalar(0, 0, 0); // Non-random
		}

		void calcCommon(bc::barline* b, double masds)
		{
			double dfs = b->matr.size() * 100.f / masds;
			if (dfs < totalMtrixCountMin)
				totalMtrixCountMin = dfs;
			if (dfs > totalMtrixCountMax)
				totalMtrixCountMax = dfs;

			// Depth
			int d = b->getDeath();
			if (d < minDepth)
				minDepth = d;
			if (d > maxDepth)
				maxDepth = d;

			if (b->start < minStart)
				minStart = b->start;
			if (b->start > maxStart)
				maxStart = b->start;

			if (b->len() < minLen)
				minLen = b->len();
			if (b->len() > maxLen)
				maxLen = b->len();
		}

		void print()
		{
			//			qDebug() << "Min d:" << minDepth << Qt::endl
			//					 << "Max d:" << maxDepth << Qt::endl
			//					 << "totalMtrixCountMin:" << totalMtrixCountMin << Qt::endl
			//					 << "totalMtrixCountMax:" << totalMtrixCountMax << Qt::endl
			//					 << "minStart:" << BackString::fromStdString(minStart.text()) << Qt::endl
			//					 << "maxStart:" << BackString::fromStdString(maxStart.text()) << Qt::endl
			//					 << "minLen:" << BackString::fromStdString(minLen.text()) << Qt::endl
			//					 << "maxLen:" << BackString::fromStdString(maxLen.text()) << Qt::endl;
		}
	};

	// Gui
	void createProject(const BackPathStr& path, const BackString& name, const BackPathStr& imgPath);

	void createBarcode(const BarcodeProperies& propertices, FilterInfo& info, int* layerId = nullptr)
	{
		if (!isImageLoaded() || mainMat.width() <= 1 || mainMat.height() <= 1)
			return;

		curSelected = nullptr;

		resultMart = mainMat;
		ska::unordered_map<size_t, char> map;

		comm.clear();
		proj->setReadyLaod(curImgInd);

		proj->createCacheBarcode(propertices, curImgInd, &info, layerId);

		created = true;
	}


	void processMain(BackString extra, FilterInfo& filter, int& layerId)
	{
		if (!created)
			return;

		//resultMart = mainMat;
		ska::unordered_map<size_t, char> map;

		comm.clear();
		proj->setReadyLaod(curImgInd);
		proj->readPrcoessBarcode(layerId, filter);
	}


	void loadImageOrProject(const BackPathStr& path);
	bool addSelectedToClassData(int classIndex, BackImage* icon = nullptr);
	void restoreSource();
	void undoAddClass();
	void exportResult(BackDirStr path);

	void save()
	{
		proj->saveProject();
	}

	int getAlg()
	{
		return proj->u_algorithm;
	}

	inline int& getTileSize() const
	{
		return proj->tileSize;
	}
	inline int& getOffsetSize()
	{
		return proj->tileOffset;
	}
	inline int getImageMinSize()
	{
 		return MIN(proj->reader->width(), proj->reader->height());
	}

	inline BackSize getImageSize()
	{
		return BackSize(proj->reader->width(), proj->reader->height());
	}

	BackDirStr getClassImagesPath()
	{
		return proj->getPath(BackPath::classfiles);
	}

	SimpleLine* getSelectedComp()
	{
		return curSelected;
	}

	SimpleLine* moveToParenr()
	{
		return curSelected = curSelected->parent.get();
	}

	void showResultPics(bool show);

	int addClassType(const BackString& name);

	std::vector<SubImgInfo> getSumImageInfos()
	{
		std::vector<SubImgInfo> info;
		if (proj->imgType != ReadType::Tiff)
			return info;

		TiffReader* treader = dynamic_cast<TiffReader*>(proj->reader);

		info = treader->getSumImageInfos();
		return info;
	}

	void setSubImage(int index)
	{
		curImgInd = index;
		proj->setCurrentSubImage(index, mainMat.width());
	}

private:
	void resetSource();
	void printCommon(int st, int ed, bool needSort);

	void endLoaded();

	//void setTempDir(const BackPathStr& path);

	//inline bc::Baritem* getBaritem()
	//{
	//	return barcode ? barcode->getItem(0) : nullptr;
	//}

	//inline bc::Baritem* getSortedBaritem()
	//{
	//	return barcode ? barcode->getItem(1) : nullptr;
	//}


private:
	// //////////////////////////////////////////////
	void deleteRange(int st, int ed, bool needSort);
	void exportAsJson(int st, int ed, bool needSort);

	//void createHolesBarcode(const bc::BarConstructor& constr, int imgIndex, int);


	// Sub
	const BackImage& getMainImage() const
	{
		return mainMat;
	}

private:
	Project* proj = nullptr;
	int imgNumber = -1;

	ComFinder comm;

	Cound** resmap = nullptr;
	std::vector<Cound*> veas;
	void maskInit();

	GuiState state = GuiState::Empty;

	BackImage mainMat;
	BackImage resultMart;
	int curImgInd;
	int curDisplayImgInd;

	//std::unique_ptr<bc::Barcontainer> barcode = nullptr;
	bool created = false;

	bc::ColorType col;
	//std::vector<Barscalar> colors;


	//BackDirStr base_root;
	//BackDirStr auto_root = base_root;
	SimpleLine* curSelected = nullptr;
	int lastIndex = 0;

	static std::string openImageOrProject();
};


bool getNumber(BackString path, int& numI, BackString& num)
{
	int del = path.find_last_of('/');
	int del2 = path.find('_', del);

	num = path.substr(del + 1, del2 - del - 1);

	char* endptr;
	numI = std::strtol(path.c_str(), &endptr, 10);
	return *endptr == '\0';
}

GuiBackend::GuiBackend()
{
	proj = Project::getProject();
	//normlPen.setColor(QColor(0, 0, 255));
	//normlPen.setWidth(1);

	//selectedPen.setColor(QColor(0, 0, 255));
	//selectedPen.setWidthF(1.5);

	//for (int s = 0, total = classer.categorues.size(); s < total; ++s)
//		qDebug() << BackString::fromStdString(classer.categorues[s]) << " : " << BackString::fromStdString(colors[s].text());

//		for (int b = 0; b < 255; b += 20)
	//			for (int g = 255; g > 20; g -= 20)
	//				for (int r = 0; r < 255; r += 100)
	//					colors.push_back(Barscalar(b, g, r));

	//BackDirStr directory(proj->getPath(BackPath::classfiles));
	//directory /= "*.json";
	//classer.addClass(createBar("D:\\Learning\\BAR\\sybery\\2.png"), 1);
}

void GuiBackend::maskInit()
{
	//if (maskImg.wid() <= 1)
	//	return;

	//bc::BarConstructor conr;
	//conr.addStructure(bc::ProcType::Radius, bc::ColorType::gray, bc::ComponentType::Component);
	//conr.maxRadius = 30;
	//conr.createBinaryMasks = true;
	//conr.createGraph = true;
	//bc::BarcodeCreator creator;
	//auto *barc = creator.createBarcode(&maskImg, conr);
	//bc::barlinevector &vec = barc->getItem(0)->getRootNode()->children;

	////	size_t len = maskImg.length();
	//for (int i = 0, total = vec.size(); i < total; ++i)
	//{
	//	if (vec[i]->start < 100)
	//	{
	//		continue;
	//	}
	//	if (vec[i]->getDeath() != 1)
	//		continue;

	//	Cound *sa = new Cound();
	//	sa->srcTotal = vec[i]->matr.size();
	//	sa->index = veas.size();
	//	veas.push_back(sa);

	//	for (int v = 0, totalv = sa->srcTotal; v < totalv; ++v)
	//	{
	//		const auto &p = vec[i]->matr[v].getPoint();
	//		int d = p.y * maskImg.wid() + p.x;
	//		assert(d < maskImg.length());
	//		resmap[d] = sa;
	//	}
	//}
	////qDebug() << "Found " << veas.size() << " mask obejcts";

	//delete barc;
}


void GuiBackend::clear()
{
	if (resmap != nullptr)
	{
		delete[] resmap;
		resmap = nullptr;
	}
	for (int i = 0, total = veas.size(); i < total; ++i)
	{
		delete veas[i];
	}
	veas.clear();
}

void GuiBackend::createProject(const BackPathStr& path, const BackString& name, const BackPathStr& imgPath)
{
	curSelected = nullptr;
	BackPathStr fullPath = path / name;
	proj->setProjectPath(fullPath);
	proj->loadImage(imgPath, 1);
	endLoaded();
	state = GuiState::ImageLoaded;
}


#define ppair(x,y,chr) (std::pair<bc::point,uchar>(bc::point(x,y), chr))

void GuiBackend::endLoaded()
{
	curDisplayImgInd = proj->getFirstNormIndex();
	curImgInd = 0;

	clear();
	proj->setReadyLaod(curImgInd);
}


void GuiBackend::loadImageOrProject(const BackPathStr& path)
{
	curSelected = nullptr;

	GuiState newState = state;
	bool setProc = false;
	if (path.extension() == ".qwr")
	{
		if (!proj->loadProject(path))
			return;
		//		return;
		setProc = true;
		newState = GuiState::BarcodeCreated;
	}
	else
	{
		proj->setProjectPath(path);
		proj->loadImage(path, 1);
		newState = GuiState::ImageLoaded;
	}

	endLoaded();
	if (setProc)
	{
		//processedImage->setImage(*proj->images[curDisplayImgInd], false);
		created = true;
	}
	else
	{
		created = false;
	}

	state = newState;
}

///////////////////////==============

void GuiBackend::exportResult(BackDirStr path)
{
	imwrite(path / "result.png", resultMart);
	//saveAllJsons(geojson, imgNumber, path);
}

void GuiBackend::restoreSource()
{
	mainMat.assignCopyOf(*proj->images[curDisplayImgInd]);
}

void GuiBackend::resetSource()
{
	mainMat.assignCopyOf(resultMart);
}

void GuiBackend::printCommon(int st, int ed, bool needSort)
{
	//auto bar = needSort ? getSortedBaritem() : getBaritem();

	//double masds = mainMat.length();

	//comm.clear();
	//for (int i = st; i < ed; ++i)
	//{
	//	auto &b = bar->barlines[i];

	//	// Matrix
	//	this->comm.calcCommon(b, masds);
	//}

	//comm.print();
}

//void GuiBackend::setTempDir(const BackPathStr& path)
//{
//	base_root = path;
//	auto_root = path;
//}

// /////////////////////////////
void swap(int& a, int& b)
{
	int temp = a;
	a = b;
	b = temp;
}

//void genLen(int *st, int *end, int max)
//{
//	*st = QRandomGenerator::global()->generate() % max;
//	*end = QRandomGenerator::global()->generate() % max;
//	if (*st > *end)
//	{
//		int temp = *st;
//		*st = *end;
//		*end = temp;
//	}
//}
//
//int rng(int st,int ed)
//{
//    return st + QRandomGenerator::global()->generate() % (ed - st);
//}


uchar maxmin(int val)
{
	if (val >= 255)
		return 255;
	if (val < 0)
		return 0;
	return val;
}


void fitInto(int sourceLen, int newLen, int& st, int& ed)
{
	st = newLen * static_cast<float>(st) / sourceLen;
	ed = newLen * static_cast<float>(ed) / sourceLen;
}
//
//void GuiBackend::createHolesBarcode(const bc::BarConstructor &constr, int imgIndex, int)
//{
//	MatrImg mat(mainMat.width(), mainMat.height(), mainMat.channels());
//
//	proj->createCacheBarcode(constr, imgIndex, mat);
//
//	resultMart.assignCopyOf(mat);
//	processedImage->setImage(resultMart);
//}


void GuiBackend::deleteRange(int st, int ed, bool needSort)
{

	//	bc::barlinevector &baselines = getBaritem()->barlines;

	//	if (needSort)
	//	{
	//		bc::barlinevector res;

	//		bc::barlinevector &todellines = getSortedBaritem()->barlines;
	//		for (size_t i = 0; i < baselines.size(); ++i)
	//		{
	//			bc::barline *baseline = baselines[i];
	//			bool found = false;
	//			for (int d = st; d < ed; ++d)
	//			{
	//				if (*baseline == *todellines[d])
	//					found = true;
	//			}
	//			if (!found)
	//				res.push_back(baseline);
	//		}
	//		baselines.clear();
	//		baselines.insert(baselines.begin(), res.begin(), res.end());
	//	}
	//	else
	//		baselines.erase(baselines.begin() + st, baselines.begin() + ed);

	//	auto bar = getBaritem()->clone();
	//	bar->sortBySize();
	//	barcode->setItem(1, bar);

	//	int size = baselines.size();
	//	QMetaObject::invokeMethod(sliderP, "setMax", Qt::AutoConnection, Q_ARG(QVariant, QVariant::fromValue(size)));
}

void GuiBackend::exportAsJson(int st, int ed, bool needSort)
{
	//	auto bar = needSort ? getSortedBaritem() : getBaritem();

	//	int barsize = bar->barlines.size();
	//	if (st >= barsize || ed > barsize)
	//	{
	//		return;
	//	}

	//	std::string jsonS = "[";
	//	for (int i = st; i < ed; ++i)
	//	{
	//		auto &b = bar->barlines[i];
	//		b->getJsonObject<BackString, toQtStr>(jsonS, false, false, true);
	//	}
	//	jsonS += "]";

	//	QFile filed("D:\\jsonOut.json");
	//	filed.write(jsonS.c_str(), jsonS.length());
	//	filed.close();
}

void GuiBackend::showResultPics(bool show)
{
	//if (show)
	//{
	//	processedImage->setImage(resultMart, false);
	//}
	//else
	//	processedImage->setImage(mainMat, false);
}

int GuiBackend::addClassType(const BackString& name)
{
	return proj->addClassType(name);
}

bool GuiBackend::addSelectedToClassData(int classIndex, BackImage* icon)
{
	if (classIndex < 0 || classIndex >= 3)
		return false;

	if (!curSelected)
		return false;

	//GeoBarCloudHolderCache reader;
	//reader.openRead();
	//auto item = std::make_unique<BarcodesHolder>(reader.loadSpecific(curSelected->id));
	//auto *curBarline = item->lines[curSelected->barlineIndex];

	//proj->addClassData(classIndex, curBarline, icon);
	//lastIndex = classIndex;
}

void GuiBackend::undoAddClass()
{
	if (lastIndex != -1)
	{
		//proj->classer.removeLast(lastIndex);
		lastIndex = -1;
	}
}
