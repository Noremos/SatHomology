// #include "DiffuseCommon.h"
// #include "Common.h"
// #include "../Interfaces/IBlock.h"

// #include "Barcode/PrjBarlib/include/barcodeCreator.h"
// #include "../MatrImg.h"
// #include "../Clusterizers/LandscapeItem.h"
// #include "../Layers/layerInterface.h"
// #include "../Layers/Rasterlayers.h"
// #include "../Layers/RasterLineLayer.h"

// // import RasterLayers;
// // import IBlock;
// // import LayersCore;
// //import IAlgorithm
#include "../backend/project.h"
// //import AlgUtils;
// //import BackBind;
// // import MatrModule;
// // import RasterLineLayerModule;


// class LandscapeItemsBlock : public IBlock
// {
// public:
// 	LandscapeItemsBlock()
// 	{
// 		IBlock::settings = { };
// 	}

// 	virtual const BackString name() const override
// 	{
// 		return "Landscape_reduce_ItemsBlock";
// 	}

// 	float doMagic(const BackImage& src, RasterLineLayer* layer)
// 	{
// 		bc::barstruct constr;// = getConstr(setting);
// 		constr.createGraph = true;
// 		constr.createBinaryMasks = true;
// 		constr.coltype = bc::ColorType::gray;
// 		constr.proctype = bc::ProcType::f255t0;

// 		auto item = (bc::BarcodeCreator::create(src, constr));
// 		size_t size = item->barlines.size();

// 		InputLandData convertLand;

// 		CachedBaritemHolder holder;
// 		TileProvider tileProv(1, 0, 0);
// 		for (size_t i = 0; i < size; ++i)
// 		{
// 			bc::barline* line = item->barlines[i];
// 			if (line == item->getRootNode())
// 				continue;
// 			if (line->start == line->end())
// 				continue;

// 			CachedBarline lb(i, line, &holder);

// 			if (layer)
// 				layer->addLine(i, &lb, tileProv);
// 			convertLand.addExpr(line->start, line->end());
// 			convertLand.lands.back().matrix = lb.getMatrix();
// 		}

// 		LandscapeCollection landscapes;
// 		landscapes.performOnPerform();
// 		landscapes.round = false;

// 		landscapes.convertToLandscape(convertLand);

// 		convertLand.lands.clear();
// 		size_t ls = landscapes.getItemsCount();

// 		return 1.0 - static_cast<double>(ls) / static_cast<double>(size);
// 	}

// 	void printStatsForFolder(std::string path)
// 	{
// 		std::vector<float> results;
// 		for (const auto& entry : std::filesystem::directory_iterator(path))
// 		{
// 			if (entry.is_regular_file())
// 			{
// 				auto ext = entry.path().extension();
// 				if (ext != ".jpg" && ext != ".JPG" && ext != ".png" && ext != ".PNG" && ext != ".bmp" && ext != ".BMP")
// 				{
// 					continue;
// 				}

// 				std::cout << "Reading " << entry.path().string() << std::endl;
// 				BackImage img = imread(entry.path());
// 				results.push_back(doMagic(img, nullptr));
// 			}
// 		}

// 		using namespace std;

// 		cout << path << endl;
// 		cout << "Mean: " << std::accumulate(results.begin(), results.end(), 0.0) / results.size() << endl;

// 		std::sort(results.begin(), results.end());
// 		cout << "Median: " << results[results.size() / 2] << endl;
// 		cout << "Max: " << *std::max_element(results.begin(), results.end()) << endl;
// 		cout << "Min: " << *std::min_element(results.begin(), results.end()) << endl;
// 	}

// 	virtual RetLayers execute(InOutLayer iol) override
// 	{
// 		Project* proj = Project::getProject();
// 		IRasterLayer* inLayer = proj->getInRaster(iol);

// 		// Line layer
// 		auto& metaprov = proj->getMeta();
// 		RasterLineLayer* layer = proj->addLayerData<RasterLineLayer>(inLayer->cs.getProjId());
// 		layer->init(inLayer, metaprov);
// 		layer->initCSFrom(inLayer->cs);
// 		layer->tileOffset = 0;

// 		BackImage src;

// 		getSrcFromInput(iol, src);

// 		bc::BarcodeCreator bcc;
// 		bc::barstruct constr;// = getConstr(setting);
// 		constr.createGraph = true;
// 		constr.createBinaryMasks = true;
// 		constr.coltype = bc::ColorType::gray;
// 		constr.proctype = bc::ProcType::f255t0;

// 		MinMax mm(src);

// 		InputLandData convertLand;

// 		auto item = (bc::BarcodeCreator::create(src, constr));
// 		size_t size = item->barlines.size();


// 		CachedBaritemHolder holder;
// 		TileProvider tileProv(1, 0, 0);
// 		for (size_t i = 0; i < size; ++i)
// 		{
// 			bc::barline* line = item->barlines[i];
// 			if (line == item->getRootNode())
// 				continue;
// 			if (line->start == line->end())
// 				continue;

// 			CachedBarline lb(i, line, &holder);

// 			layer->addLine(i, &lb, tileProv);
// 			convertLand.addExpr(line->start, line->end());
// 			convertLand.lands.back().matrix = lb.getMatrix();
// 		}

// 		LandscapeCollection landscapes;
// 		landscapes.performOnPerform();
// 		landscapes.round = false;

// 		landscapes.convertToLandscape(convertLand);

// 		convertLand.lands.clear();
// 		size_t ls = landscapes.getItemsCount();

// 		RasterLineLayer* layer2 = proj->addLayerData<RasterLineLayer>(inLayer->cs.getProjId());
// 		layer2->init(inLayer, metaprov);
// 		layer2->initCSFrom(inLayer->cs);
// 		layer2->tileOffset = 0;

// 		RasterLayer* layer3 = proj->addLayerData<RasterLayer>(inLayer->cs.getProjId());
// 		layer3->init(inLayer);
// 		layer3->initCSFrom(inLayer->cs);
// 		layer3->tileOffset = 0;

// 		for (size_t i = 0; i < ls; i++)
// 		{
// 			const auto& matr = landscapes.getCItem(i)->getMatrix();
// 			for (auto &v : matr)
// 			{
// 				layer3->mat.add(v.x, v.y, v.value);
// 				layer3->mat.setOpp(v.x, v.y, 255);
// 			}

// 			layer2->addLine(i, landscapes.getCItem(i), tileProv);
// 		}

// 		std::cout << "Source bars size: " << static_cast<double>(size) << std::endl;
// 		std::cout << "Bars size after : " << static_cast<double>(ls) << std::endl;
// 		std::cout << 1.0 - static_cast<double>(ls) / static_cast<double>(size) << std::endl;


// 		// printStatsForFolder("/Users/sam/Edu/bar/12");
// 		printStatsForFolder("/Users/sam/Edu/bar/test/base");
// 		printStatsForFolder("/Users/sam/Edu/datasets/faces/faces/7");

// 		// /Users/sam/Edu/bar/12
// 		// //Users/sam/Edu/bar/test/base
// 		// /Users/sam/Edu/datasets/faces/faces/7
// 		return {layer, layer2, layer3};
// 	}

// 	void clear() override
// 	{
// 	}

// 	void addInput(InOutLayer iol) override
// 	{
// 		// Project* proj = Project::getProject();
// 		addToTrain(iol);
// 		// proj->saveProject();
// 	}


// 	void addToTrain(InOutLayer iol)
// 	{

// 	}


// };

// BlockRegister<LandscapeItemsBlock> regLandscapeItems;
