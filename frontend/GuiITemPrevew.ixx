export module GuiITemPrevew;
import LayersCore;
import RasterBarHolderRLayer;
import ProjectModule;
import Settings;

export RetLayers makePreviwLayer(RasterLineLayer* inLayer, int methodId)
{
	// Input Layer prepatons
	int tileSize = inLayer->prov.tileSize;
	int tileOffset = inLayer->tileOffset;
	SubImgInf curSize = inLayer->getSubImgInf(); // Cursubimg

	// End Input Layer

	// Setup output layers
	//
	LayerProvider& prov = inLayer->prov;
	// Line layer
	auto& metaprov = getSettings().getMeta();
	RasterLineLayer* layer = Project::getProject()->addLayerData<RasterLineLayer>(inLayer->cs.getProjId());
	layer->init(inLayer, metaprov);
	layer->initCSFrom(inLayer->cs);
	layer->tileOffset = tileOffset;


	RetLayers ret;
	ret.push_back(layer);

/*
	prov.init(curSize.wid, curSize.hei, inLayer->displayWidth(), tileSize);
	TileProvider tileProv = prov.tileByOffset(offset.x, offset.y);
	

	auto collectionToPredict = getClusterFactory().CreateHolder(methodId);
	inLayer->collectionToPredict = collectionToPredict.get();
	inLayer->processCachedBarcode(nullptr, false);
	//for
	IdGrater parentne;
	int inde = 0;
	for (size_t i = 0; i < collectionToPredict->getItemsCount(); i++)
	{

		layer->addLine(parentne, inde++, collectionToPredict->getItem(), tileProv);
	}
	*/

	//
	// Classes layers
	//for (auto& i : classLayers)
	//{
	//	ret.push_back(i.second);
	//	i.second->clear();
	//	i.second->color = classCategs.get(i.first)->color;
	//	i.second->initCSFrom(inLayer->cs);
	//}
	// -------------------

	// Cacher
	//ItemHolderCache cacher;
	//cacher.openWrite(layer->getCacheFilePath(prov));
	// ------


	// Setup tileIterators
	//TileImgIterator tileIter(tileSize, tileOffset, curSize.wid, curSize.hei);

	// Threads

	return ret;
}
