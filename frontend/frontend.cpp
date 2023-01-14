#include "frontend.h"
#include "../backend/geodataprocessor.h"
#include "../side/sago/platform_folders.h"
#include <unordered_set>

barclassificator classer;
bc::barlinevector geojson[3];


bool getNumber(BackString path, int &numI, BackString& num)
{
	int del = path.find_last_of('/');
	int del2 = path.find('_', del);

	num = path.substr(del + 1, del2 - del - 1);

	char* endptr;
	numI = std::strtol(path.c_str(), &endptr, 10);
	return *endptr == '\0';
}


bc::Baritem *createBar(BackString path)
{
	MatrImg img = imread(path);

	bc::BarConstructor constr;
	constr.createBinaryMasks = false;
	constr.createGraph = false;
	constr.attachMode = bc::AttachMode::morePointsEatLow;
	constr.returnType = bc::ReturnType::barcode2d;
	constr.addStructure(bc::ProcType::f0t255, bc::ColorType::rgb, bc::ComponentType::Component);

	bc::BarcodeCreator bc;
	auto *barc = bc.createBarcode(&img, constr);
	auto *item = barc->exractItem(0);
	delete barc;

	return item;
}

GuiBackend::GuiBackend()
{
	proj = Project::getProject();
	//normlPen.setColor(QColor(0, 0, 255));
	//normlPen.setWidth(1);

	//selectedPen.setColor(QColor(0, 0, 255));
	//selectedPen.setWidthF(1.5);


	srand(300);
	colors.push_back(Barscalar(255, 0, 0));
	colors.push_back(Barscalar(0, 255, 0));
	colors.push_back(Barscalar(0, 0, 255));
	for (int k = 0; k < 40; ++k)
		colors.push_back(Barscalar(5 +  rand() % 250, 5 + rand() % 250, 5 + rand() % 250));

	colors.push_back(Barscalar(255, 255, 255));

	//for (int s = 0, total = classer.categorues.size(); s < total; ++s)
//		qDebug() << BackString::fromStdString(classer.categorues[s]) << " : " << BackString::fromStdString(colors[s].text());

//		for (int b = 0; b < 255; b += 20)
	//			for (int g = 255; g > 20; g -= 20)
	//				for (int r = 0; r < 255; r += 100)
	//					colors.push_back(Barscalar(b, g, r));

	classer.categorues.push_back("bad");
	classer.categorues.push_back("roof двускатные");
	classer.categorues.push_back("roof односкатные");

	//BackDirStr directory(proj->getPath(BackPath::classfiles));
	//directory /= "*.json";

	BackPathStr directory = std::filesystem::path(sago::getDocumentsFolder());
	directory /= "Qbinbar";

	if (!pathExists(directory))
		mkdir(directory);

	for (auto const& entry : std::filesystem::directory_iterator(directory))
	{
		if (!entry.is_regular_file() || entry.path().extension() != ".json")
			continue;

		auto filename = entry.path().filename().string();
		//do whatever you need to do
		int del2 = filename.find('_');
		BackString num = filename.substr(0, del2);
		bool ok;
		int numi = strToInt(num, ok);
		assert(ok);
		classer.addClass(auto_root / filename, numi);
	}

	classer.addClass(createBar("D:\\Learning\\BAR\\sybery\\2.png"), 1);


	setTempDir(directory);
}

void GuiBackend::maskInit()
{
	if (maskImg.wid() <= 1)
		return;

	bc::BarConstructor conr;
	conr.addStructure(bc::ProcType::Radius, bc::ColorType::gray, bc::ComponentType::Component);
	conr.maxRadius = 30;
	conr.createBinaryMasks = true;
	conr.createGraph = true;
	bc::BarcodeCreator creator;
	auto *barc = creator.createBarcode(&maskImg, conr);
	bc::barlinevector &vec = barc->getItem(0)->getRootNode()->children;

	//	size_t len = maskImg.length();
	for (int i = 0, total = vec.size(); i < total; ++i)
	{
		if (vec[i]->start < 100)
		{
			continue;
		}
		if (vec[i]->getDeath() != 1)
			continue;

		Cound *sa = new Cound();
		sa->srcTotal = vec[i]->matr.size();
		sa->index = veas.size();
		veas.push_back(sa);

		for (int v = 0, totalv = sa->srcTotal; v < totalv; ++v)
		{
			const auto &p = vec[i]->matr[v].getPoint();
			int d = p.y * maskImg.wid() + p.x;
			assert(d < maskImg.length());
			resmap[d] = sa;
		}
	}
	//qDebug() << "Found " << veas.size() << " mask obejcts";

	delete barc;
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

void GuiBackend::settup(GuiImage *mainImage, GuiImage* processedImage, GuiItem* sliderPanel)
{
	this->mainImage = mainImage;
	this->processedImage = processedImage;
    //    QtCharts::QChart *obj = barchartP->findChild<QtCharts::QChart *>("barchart");
	this->sliderP = sliderPanel;
}

void GuiBackend::createBarcode(bc::ProcType procType, bc::ColorType colType, bc::ComponentType compType)
{
	if (!isImageLoaded() || mainMat.width() <= 1 || mainMat.height() <= 1)
		return;

	bc::BarcodeCreator bc;
	bc::BarConstructor constr;
	constr.createBinaryMasks = true;
	constr.createGraph = true;
	constr.attachMode = bc::AttachMode::morePointsEatLow;
	constr.returnType = bc::ReturnType::barcode2d;
	constr.addStructure(procType, colType, compType);
//	constr.setStep(stepSB);

	barcode.reset(proj->createBarcode(constr, curImgInd, 0));
//	createBarcode(constr, curImgInd, 0);
//	barcode.reset(bc.createBarcode(&mainMat, constr));

//	auto bar = getBaritem()->clone();
//	for (int i = 0; i < bar->barlines.size(); ++i)
//	{
//		bar->barlines[i]->getDeath();
//	}


//	bar->sortBySize();
//	barcode->addItem(bar);

//	int ma = getBaritem()->barlines.size();
//	QMetaObject::invokeMethod(sliderP, "setMax", Qt::AutoConnection, Q_ARG(QVariant, QVariant::fromValue(ma)));

	processedImage->setImage(mainMat);
	created = true;
}

#define ppair(x,y,chr) (std::pair<bc::point,uchar>(bc::point(x,y), chr))

void GuiBackend::loadImageOrProject(const BackPathStr& path)
{
    //if (root == nullptr)
		//return;

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

	curImgInd = curDisplayImgInd = proj->getFirstNormIndex();
//	curImgInd = 0;
	mainMat.assignCopyOf(*proj->images[curDisplayImgInd]);
	clearResLine();
	initResLine(mainMat.length());

	//mainImage->setSource(proj->getTilePath(curDisplayImgInd));
	//imwrite(BackString("D:\\32.png"), mainMat);
	mainImage->setImage(mainMat);

	maskImg = MatrImg(1, 1, 1);

	if (setProc)
	{
		processedImage->setImage(*proj->images[curDisplayImgInd]);
		created = true;
	}
	else
	{
		created = false;
	}

	clear();
	resmap = new Cound*[maskImg.length()];
	memset(resmap, 0, maskImg.length() * sizeof(Cound *));

	proj->setCurrentSubImage(curImgInd);
	factor = (float)proj->reader->widght() / mainMat.width();
	proj->u_displayFactor = factor;

	state = newState;

//	const QStringList colorList = {"red",
//								   "green",
//								   "blue",
//								   "yellow"};

//	const QStringList moduleList = {"Core", "GUI", "Multimedia", "Multimedia Widgets", "Network",
//									"QML", "Quick", "Quick Controls", "Quick Dialogs",
//									"Quick Layouts", "Quick Test", "SQL", "Widgets", "3D",
//									"Android Extras", "Bluetooth", "Concurrent", "D-Bus",
//									"Gamepad", "Graphical Effects", "Help", "Image Formats",
//									"Location", "Mac Extras", "NFC", "OpenGL", "Platform Headers",
//									"Positioning", "Print Support", "Purchasing", "Quick Extras",
//									"Quick Timeline", "Quick Widgets", "Remote Objects", "Script",
//									"SCXML", "Script Tools", "Sensors", "Serial Bus",
//									"Serial Port", "Speech", "SVG", "UI Tools", "WebEngine",
//									"WebSockets", "WebView", "Windows Extras", "XML",
//									"XML Patterns", "Charts", "Network Authorization",
//									"Virtual Keyboard", "Quick 3D", "Quick WebGL"};

//	QList<QObject *> dataList;
//	for (const BackString &module : moduleList)
//		dataList.append(new DataObject("Qt " + module, colorList.at(rand() % colorList.length())));

//	QQuickView view;
//	view.setResizeMode(QQuickView::SizeRootObjectToView);
//	view.setInitialProperties({{ "model", QVariant::fromValue(dataList) }});
}

int GuiBackend::getBarsCount()
{
	return (int)barcode->getItem(0)->barlines.size();
}


///////////////////////==============

void GuiBackend::exportResult(BackDirStr path)
{
	imwrite(path / "result.png", resultMart);
	saveAllJsons(geojson, imgNumber, path);
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
	auto bar = needSort ? getSortedBaritem() : getBaritem();

	double masds = mainMat.length();

	comm.clear();
	for (int i = st; i < ed; ++i)
	{
		auto &b = bar->barlines[i];

		// Matrix
		this->comm.calcCommon(b, masds);
	}

	comm.print();
}

void GuiBackend::setTempDir(const BackPathStr& path)
{
	base_root = path;
	auto_root = path;
}

// /////////////////////////////
void swap(int &a, int &b)
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

#include "../backend/MatrImg.h"
#include "../side/PortFileDialog.h"
#include <random>


void fitInto(int sourceLen, int newLen, int &st, int &ed)
{
	st = newLen * static_cast<float>(st) / sourceLen;
	ed = newLen * static_cast<float>(ed) / sourceLen;
}

void GuiBackend::createHolesBarcode(const bc::BarConstructor &constr, int imgIndex, int)
{
	ImageReader *reader = proj->reader;
	//	reader->setCurrentSubImage(1);
	proj->setCurrentSubImage(imgIndex);

	factor = (float)proj->reader->widght() / mainMat.width();
	proj->u_displayFactor = factor;

	bc::BarcodeCreator creator;

	const uint offts = 100;
	const uint fullTile = proj->tileSize + offts;

	//	FileBuffer boundStream;
	//	if (!boundStream.openFileStream(getPath(BackPath::barlist)))
	//		return barconsrt;

	uint rwid = reader->widght();
	uint rhei = reader->height();

	BarBinFile::sets set;
	set.totalSize = rwid * rhei;

	MatrImg mat(mainMat.width(), mainMat.height(), mainMat.channels());

	std::unordered_map<size_t, char> map;

	int ke = 0;
	for (uint stH = 0; stH < rhei; stH += proj->tileSize)
	{
		uint ihei = (stH + fullTile > rhei ? rhei - stH : fullTile);
		//qDebug() << stH;
		for (uint stW = 0; stW < rwid; stW += proj->tileSize)
		{
			uint iwid = (stW + fullTile > rwid ? rwid - stW : fullTile);

			DataRect rect = reader->getRect(stW, stH, iwid, ihei);
			//			for (uint rX = 0; rX < iwid; ++rX)
			//			{
			//				for (uint rY = 0; rY < ihei; ++rY)
			//				{
			//					const auto v = rect.get(rX, rY);
			//					const auto* p = v.val.rgba.samples;
			//					remp.setPixel(stW + rX, stH + rY, qRgb(p[0].s,p[1].s,p[2].s));
			//				}
			//			}
			//			remp.save("D:\\temp.png");

			DataRectBarWrapper warp(rect);
			auto *ret = creator.createBarcode(&warp, constr);
//			BarcodeHolder holderT = proj->threasholdLines(ret->getItem(0));
			BarcodesHolder holder = proj->toHoldes(ret->getItem(0)->barlines, mat, bc::point(stW, stH));

			int ind = ke++;
			//qDebug() << "Crc" << ind;
			classBarcode(holder, ind, mat, map, "");
			delete ret;
		}
	}

	resultMart.assignCopyOf(mat);
	processedImage->setImage(resultMart);
}

MatrImg GuiBackend::mask(BarBinFile *bar, MatrImg &mat, BackString& extra)
{
	//MatrImg outMask(mat.wid(), mat.hei(), 1);
	MatrImg outMask(1,1, 1);
	outMask.fill(0);
	mat.assignCopyOf(mainMat);
	double masds = mat.length();

	curSelected = nullptr;
	lastIndex = -1;

	std::unordered_map<size_t, char> map;
	bc::barlinevector resLines;

//	for (int i = 0, total = bar->barlines.size(); i < total; ++i)
//	{
//		auto b = bar->barlines[i];
//		if (!checkLmd(b))
//			continue;

//		if (b->getDeath() > 4)
//			continue;
////		if (b->len() < Barscalar(5, 5, 5))
////			continue;
//		if (b->start < Barscalar(16, 16, 16)) //  Barscalar(5, 5, 5)
//			continue;

//		resLines.push_back(b);
//		map[reinterpret_cast<size_t>(b)] = -2;
//	}

	int total = resLines.size();
	int rTotal = total;


	bool wr = extra.find("json;") != -1;
//	bool ent = extra.indexOf("entr;") != -1;
//	bool classif = extra.indexOf("barclass;") != -1;
	bool showBad = extra.find("show0;") != -1;
	bool showWhite = extra.find("showw;") != -1;

	geojson[0].clear();
	geojson[1].clear();
	geojson[2].clear();

//	fitInto(bar->barlines.size(), rTotal, st, ed);
	clearResLine();

	if (factor < 1.0)
		throw std::exception();

	while (!bar->ended())
	{
		int ind = 0;
		BarcodesHolder baritem(bar->read(ind));
		std::cout << ind << std::endl;

		uint xOff = 0;
		uint yOff = 0;
		proj->getOffsertByTileIndex(ind, xOff, yOff);
		BarcodesHolder holder = proj->toHoldes(baritem, mat, bc::point(xOff, yOff));
//		classBarcode(holder, ind, mat, map, extra);

		if (bar->memoffs.size() == ind + 1)
			break;
	}

	bar->close();

	return outMask;
}


void GuiBackend::classBarcode(BarcodesHolder& baritem, int ind, MatrImg &mat, std::unordered_map<size_t, char> &map, BackString extra)
{
	bool wr = extra.find("json;") != -1;
	//	bool ent = extra.indexOf("entr;") != -1;
	//	bool classif = extra.indexOf("barclass;") != -1;
	bool showBad = true;
	//extra.indexOf("show0;") != -1;
	bool showWhite = true; //extra.indexOf("showw;") != -1;

	uint xOff = 0;
	uint yOff = 0;
	proj->getOffsertByTileIndex(ind, xOff, yOff);

	auto &vec = baritem.lines;
	for (size_t i = 0; i < vec.size(); ++i)
	{
		auto *b = vec.at(i);
		const auto &matr = b->matrix;

		if (matr.size() == 0)
			continue;

		int type = -1;
		{
			// Its clone inside
//			type = classer.getType(b);
			//				tlines = std::move(item.barlines);
		}

		//			if (type > 0)
		//				geojson[type].push_back(b);

//		if (type <= 0 && !showBad)
//			continue;

		if (type == -1)
			type = rand() % colors.size();

		int depth = b->getDeath();
//		if (depth == 1)
//			continue;
//		if (showWhite && type == 0)
//			continue;

		Barscalar color = colors[type];

		std::unordered_set<uint> vals;

		std::shared_ptr<SimpleLine> sl = std::make_shared<SimpleLine>(ind, i);
		sl->depth = depth;
//		simpleHolder.push_back(sl);

		map[(size_t)sl.get()] = type;

		for (const auto &pm : matr)
		{
			int ox = xOff + pm.getX();
			int oy = yOff + pm.getY();
			int x = (ox) / factor;
			int y = (oy) / factor;
			uint index = bc::barvalue::getStatInd(x, y);
			if (vals.find(index) != vals.end())
				continue;

			bc::point p = bc::barvalue::getStatPoint(index);
			sl->matr.push_back(bc::barvalue(p, pm.value));

			mat.set(x, y, color);
//			outMask.set(x, y, 255);
			int indLocal = (y * mat.wid() + x);
			SimpleLine* temp = resLinesMap[indLocal].get();
//			continue;
			if (temp == nullptr || /*map[(size_t)temp] == -2 ||*/ temp->getDeath() < b->getDeath())
			{
				resLinesMap[indLocal] = sl;
//				++sl->counter;
			}
		}

	}
}

void GuiBackend::processMain(BackString extra)
{
	if (!created)
		return;

	MatrImg mat(mainMat.width(), mainMat.height(), mainMat.channels());

//	auto bar = needSort ? getSortedBaritem() : getBaritem();

//	int barsize = bar->barlines.size();
//	if (st >= barsize || ed > barsize)
//	{
//		return;
//	}

	comm.clear();
	proj->setCurrentSubImage(curImgInd);

	factor = (float)proj->reader->widght() / mainMat.width();
	proj->u_displayFactor = factor;
	BarBinFile reader;
	reader.openRead(proj->getPath(BackPath::binbar).string());
	mask(&reader, mat, extra);

	comm.print();

	resultMart.assignCopyOf(mat);
	processedImage->setImage(resultMart);
}


void GuiBackend::deleteRange(int st, int ed, bool needSort)
{
	if (!barcode)
		return;

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

void GuiBackend::click(int x, int y, BackString extra)
{
	if (mainImage == nullptr || resLinesMap.size() == 0)
		return;

	int ind = extra.find("_addclass;");
	int ttype = -1;
	if (ind != -1)
	{
		int ind2 = extra.find(';', ind - 2);
		BackString type = extra.substr(ind2 + 1, ind - ind2 - 1);
		ttype = strToInt(type);
	}

	x = mainImage->getRealX(x);
	y = mainImage->getRealY(y);

	SimpleLine *line = resLinesMap[y * mainMat.wid() + x].get();
	if (line)
	{
		if (curSelected == line && line->parent)
			line = line->parent;

		curSelected = line;
		std::cout << line->matr.size() * 100.f / mainMat.length() << std::endl;
		MatrImg temp;
		temp.assignCopyOf(resultMart);
		auto &fullmatr = line->matr;
//		line->getChildsMatr(fullmatr);

		mcountor cont;
		getCountour(fullmatr, cont, false);
		for (int i = 0, total = cont.size(); i < total; ++i)
		{
			auto p = bc::barvalue::getStatPoint(cont[i]);
			temp.set(p.x, p.y, Barscalar(255, 191, 0));
		}

		if (ttype != -1)
		{
			addClass(ttype);
		}

		resultMart = temp;
		processedImage->setImage(resultMart);
	}
}

void GuiBackend::addClass(int classIndex)
{
	if (classIndex < 0 || classIndex >= N)
		return;

	std::string jsonS;

	BarBinFile reader;
	reader.openRead(proj->getPath(BackPath::binbar).string());
	std::unique_ptr<BarcodeHolder> item(reader.readItem(curSelected->id));
	auto *curBarline = item->lines[curSelected->barlineIndex];
	curBarline->getJsonObject<std::string, toStdStr>(jsonS, bc::barline::ExportGraphType::exportLines, false, true);
	BackString name, suff;
	name = intToStr(classIndex);
	BackPathStr doorPat = auto_root;
	std::mt19937 generator(std::random_device{}());
	std::uniform_int_distribution<int> distribution(1, 1000);

	do
	{
		suff = std::format("{}_{}.json", name, distribution(generator));
	} while (pathExists(doorPat / suff));

	doorPat /= suff;

	std::ofstream file(doorPat, std::ios::trunc);
	if (file.is_open())
	{
		classer.addClass(curBarline, classIndex);
		file.write(jsonS.c_str(), jsonS.length());
		file.close();
		lastIndex = classIndex;
	}
}

void GuiBackend::undoAddClass()
{
	if (lastIndex != -1)
	{
		classer.removeLast(lastIndex);
		lastIndex = -1;
	}
}


