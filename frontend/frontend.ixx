module;

#include <unordered_set>
#include <random>
#include <memory>

#include "PortFileDialog.h"
#include "sago/platform_folders.h"

#include "../frontend/DrawCommon.h"

export module FrontendBind;

import ProjectModule;
import Platform;
import IOCore;
import BarcodeModule;
import RasterLayers;

import GuiWidgets;
import IItemModule;
import VectorLayers;
import CSBind;
import LayersCore;
import MetadataIOCore;
import BarTypes;
import BarScalarModule;
import RasterBarHolderRLayer;
// Todo.
// 2 режима
export class GuiBackend
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
		proj->loadImage(imgPath, 1);
		proj->state = GuiState::Loaded;
	}


	RetLayers createCacheBarcode(InOutLayer& iol, const BarcodeProperies& propertices, IItemFilter* filter = nullptr)
	{
		//if (block) return {};
		IRasterLayer* inLayer = Project::proj->getInRaster(iol);

		RasterLineLayer* layer = Project::proj->addOrUpdateOut<RasterLineLayer>(iol, inLayer->cs.getProjId());
		auto layers = layer->createCacheBarcode(inLayer, propertices, filter);

		//u_algorithm = propertices.alg;
		proj->saveProject();


		return layers;
	}


	//RetLayers processRaster(InOutLayer& layer, IItemFilter* filter)
	//{
	//	if (!created)
	//		return RetLayers();

	//	return proj->processCachedBarcode(layer, filter);
	//}


	RetLayers exeFilter(InOutLayer& layer, bc::ProcType type, int algNum)
	{
		if (!isLoaded())
			return RetLayers();

		return proj->exeFilter(layer, type, algNum);
	}


	RetLayers exeGUI(InOutLayer& layer, const BarcodeProperies& propertices, IItemFilter* filter)
	{
		if (!isLoaded())
			return RetLayers();

		return proj->exeGUI(layer, propertices, filter);
	}

	VectorLayer* addVectorLayer()
	{
		return proj->addLayerData<VectorLayer>();
	}

	void removeLayer(uint id)
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
			layer = proj->loadImage(path, 1);
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
	int imgNumber = -1;

	GuiState state = GuiState::Empty;

	//int curImgInd;
	//int curDisplayImgInd;

	//std::unique_ptr<bc::Barcontainer> barcode = nullptr;

	bc::ColorType col;
	int lastIndex = 0;
};

int GuiBackend::opens = 0;
