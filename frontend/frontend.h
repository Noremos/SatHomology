// module;

#pragma once
#include <unordered_set>
#include <random>
#include <memory>

#include "PortFileDialog.h"
#include "sago/platform_folders.h"

#include "../frontend/DrawCommon.h"
#include "Barcode/PrjBarlib/include/barscalar.h"
#include "Barcode/PrjBarlib/include/barline.h"
#include "Barcode/PrjBarlib/include/barcodeCreator.h"


#include "../backend/CSBind.h"
#include "../backend/Interfaces/IItem.h"
#include "../Bind/Framework.h"
#include "../backend/Layers/layerInterface.h"
#include "../backend/Layers/Rasterlayers.h"
#include "../backend/Layers/RasterLineLayer.h"
// export module FrontendBind;

//import IAlgorithm
#include "../backend/project.h"
// import Platform;
// import BarcodeModule;
// import RasterLayers;

//import GuiWidgets;
#include "GuiWidgets.h"
// import IItemModule;
//import VectorLayers;
#include "../backend/Layers/VectorLayers.h"
// import CSBind;
// import LayersCore;
// import MetadataCoreIO;
// import BarTypes;
// import BarScalarModule;
// import RasterLineLayerModule;
//import VectorLayers;
#include "../backend/Algs/CoreLoaders.h"
//import BackBind;

// Todo.
// 2 режима
MEXPORT class GuiBackend
{
	static int opens;

public:
	Project* proj = nullptr;
	GuiBackend()
	{
		proj = Project::getProject();
		opens++;
	}
	~GuiBackend()
	{
		opens--;
		if (opens == 0)
			Project::dropProject();
	}

	DisplaySystem& getDS()
	{
		return proj->getDisplay();
	}

	bool isLoaded() const
	{
		return proj->state >= GuiState::Loaded;
	}

	MetadataProvider& getMeta()
	{
		return proj->getMeta();
	}

	// Gui
	void createProject(const BackPathStr& path, const BackString& name, const BackPathStr& imgPath)
	{
		BackPathStr fullPath = path / name;
		proj->setProjectPath(fullPath);
		dropDirIfExists(proj->getPath(BackPath::metadata));
		loadImage(imgPath, 1);
		proj->state = GuiState::Loaded;
	}

	//RetLayers processRaster(InOutLayer& layer, IItemFilter* filter)
	//{
	//	if (!created)
	//		return RetLayers();

	//	return proj->processCachedBarcode(layer, filter);
	//}

	VectorLayer* addVectorLayer()
	{
		return proj->addLayerData<VectorLayer>();
	}

	void removeLayer(buint id)
	{
		proj->layers.remove(id);
	}

	RasterFromDiskLayer* loadImageOrProject(const BackPathStr& path)
	{
		RasterFromDiskLayer* layer = nullptr;
		GuiState newState = proj->state;
		bool setProc = false;
		if (path.extension() == ".qwr")
		{
			if (!proj->loadProject(path))
				return nullptr;
			//		return;
			setProc = true;
			newState = GuiState::Loaded;
		}
		else
		{
			if (state == GuiState::Empty)
			{
				proj->setProjectPath(path, true);
			}

			newState = GuiState::Loaded;
			layer = loadImage(path, 1);
		}


		proj->state = newState;
		return layer;
	}


	void save()
	{
		proj->saveProject();
	}


private:


private:

private:
	GuiState state = GuiState::Empty;
};
