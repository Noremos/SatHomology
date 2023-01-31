#ifndef FRONTEND_H
#define FRONTEND_H

#include "barcodeCreator.h"
#include "../backend/MatrImg.h"
#include "../backend/project.h"
#include "../frontend/Framework.h"
#include "../frontend/GuiCommon.h"
#include "../frontend/GuiWidgets.h"


// Todo.
// 2 режима
class GuiBackend
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
		clearResLine();

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

	void settup(GuiDrawImage* mainImage, GuiDrawImage* processedImage, GuiItem* sliderPanel);
	void loadImageOrProject(const BackPathStr& path);
	void createBarcode(bc::ProcType procType, bc::ColorType colType, bc::ComponentType compType, const FilterInfo& info);
	bool addSelectedToClassData(int classIndex, BackImage* icon = nullptr);
	void processMain(BackString extra);
	void restoreSource();
	void undoAddClass();
	void exportResult(BackDirStr path);

	void save()
	{
		proj->saveProject();
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
 		return std::min(proj->reader->widght(), proj->reader->height());
	}

	inline BackSize getImageSize()
	{
		return BackSize(proj->reader->widght(), proj->reader->height());
	}

	BackDirStr getClassImagesPath()
	{
		return proj->getPath(BackPath::classfiles);
	}

	bc::barvector* click(int x, int y);

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
	GuiItem* root = nullptr;
	GuiDrawImage* mainImage = nullptr;
	//std::atomic<GuiDrawImage*> processedImage = nullptr;
	GuiDrawImage* processedImage = nullptr;
	GuiItem* sliderP = nullptr;

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
	std::vector<std::shared_ptr<SimpleLine>> resLinesMap;
	//	std::vector<SimpleLine*> simpleHolder;
	int lastIndex = 0;

	void initResLine(int size)
	{
		resLinesMap.resize(size);
	}

	void clearResLine()
	{
		//		memset(resLinesMap, 0, mainMat.length() * sizeof(SimpleLine *));
		//		for (size_t i = 0; i < simpleHolder.size(); ++i)
		//		{
		//			delete simpleHolder[i];
		//		}
		//		simpleHolder.clear();
		resLinesMap.clear();
	}

	static std::string openImageOrProject();
};

#endif // FRONTEND_H
