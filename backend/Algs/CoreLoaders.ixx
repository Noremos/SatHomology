module;
#include "Common.h"

#include "../Interfaces/IItem.h"
#include "../Layers/layerInterface.h"
#include "../Layers/Rasterlayers.h"
#include "../Layers/RasterLineLayer.h"
export module CoreLoaders;
//import BackBind;
// import LayersCore;
// import RasterLineLayerModule;
// import RasterLayers;
import ProjectModule;
// import IItemModule;


export RetLayers createCacheBarcode(InOutLayer& iol, const BarcodeProperies& propertices, IItemFilter* filter = nullptr)
{
	Project* proj = Project::proj;

	IRasterLayer* inLayer = proj->getInRaster(iol);

	RasterLineLayer* layer = proj->addOrUpdateOut<RasterLineLayer>(iol, inLayer->cs.getProjId());
	auto ret = layer->createCacheBarcode(inLayer, propertices, filter);

	proj->saveProject();

	return ret;
}




export RasterFromDiskLayer* loadImage(const BackPathStr& path, int step)
{
	Project* proj = Project::proj;

	RasterFromDiskLayer* layer = proj->addLayerData<RasterFromDiskLayer>();
	layer->open(path, proj->getMeta());

	proj->saveProject();

	return layer;
}
