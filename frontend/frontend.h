#ifndef FRONTEND_H
#define FRONTEND_H

#include "barcodeCreator.h"
#include "../backend/MatrImg.h"
#include "../backend/project.h"
#include "../frontend/Framework.h"
#include "../frontend/GuiCommon.h"

// Todo.
// 2 режима
class GuiBackend
{
	GuiBackend();
	~GuiBackend()
	{
		clearResLine();

		clear();
		Project::dropProject();
	}

	Project* proj;
	float factor;

	void clear();
	int imgNumber = -1;

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

	ComFinder comm;

	Cound** resmap = nullptr;
	std::vector<Cound*> veas;
	void maskInit();


	GuiItem* root = nullptr;
	GuiImage* mainImage = nullptr;
	GuiImage* processedImage = nullptr;
	GuiItem* sliderP = nullptr;

	MatrImg mainMat;
	MatrImg sourceBack;
	MatrImg resultMart;
	MatrImg maskImg;
	int curImgInd;
	int curDisplayImgInd;

	BackImage result;
	std::unique_ptr<bc::Barcontainer> barcode = nullptr;
	bool created = false;

	bc::ColorType col;
	std::vector<Barscalar> colors;

	void settup(GuiImage* mainImage, GuiImage* processedImage, GuiItem* sliderPanel);
	void run(int procType, int colType, int compType);
	void loadImage(BackString path);
	int getBarsCount();
	void exportResult(BackDirStr path);
	void restoreSource();
	void resetSource();
	void printCommon(int st, int ed, bool needSort);
	void setTempDir(const BackPathStr& path);

	inline bc::Baritem* getBaritem()
	{
		return barcode ? barcode->getItem(0) : nullptr;
	}

	inline bc::Baritem* getSortedBaritem()
	{
		return barcode ? barcode->getItem(1) : nullptr;
	}

	// //////////////////////////////////////////////
	void processMain(GuiImage* iamge, int st, int ed, int mode, bool needSort, BackString extra);
	void deleteRange(int st, int ed, bool needSort);
	void exportAsJson(int st, int ed, bool needSort);
	void click(int x, int y, BackString extra);
	void addClass(int classIndex);
	void undoAddClass();

	void classBarcode(BarcodesHolder& baritem, int ind, MatrImg& mat, std::unordered_map<size_t, char>& map, BackString extra);
	void createBarcode(const bc::BarConstructor& constr, int imgIndex, int);


private:
	BackDirStr base_root;
	BackDirStr auto_root = base_root;
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
	MatrImg mask(BarBinFile* bar, MatrImg& mat, int st, int ed, BackString& extra);

	//
	static std::string openImageOrProject();
};

#endif // FRONTEND_H
