module;
#include "GuiCommon.h"

export module LayersGui;

import RasterLayers;
import ProjectModule;
import BarcodeModule;
import GuiWidgets;
import GuiOverlap;
import IOCore;
import VectorLayers;
import CSBind;
//import ForntnedModule;

class LayersVals;
class ILayerWorker;

Project* proj = Project::getProject();
//GuiBackend backend;

export class IGuiLayer
{
protected:
	BackString strId;
	int copiedId = -1; // We need to cpy Id to avoid cases when core was destroid
public:
	bool visible = true;

	virtual void draw(const GuiDisplaySystem& ds) = 0;
	virtual const char* getName() const = 0;
	virtual void setName(const BackString& name, bool updateOnlyEmpty = false) = 0;

	virtual GuiImage* getIcon() = 0;

	virtual void toGuiData() = 0;

	virtual ILayer* getCore() = 0;

	virtual void drawOverlap(const GuiDisplaySystem&)
	{ }

	virtual void drawToolbox(ILayerWorker& context) = 0;

	virtual void drawProperty() = 0;
	virtual void applyPropertyChanges() = 0;

	void lockAtThis()
	{
		auto* ccore = getCore();
		DisplaySystem& ds = proj->getDisplay();
		ds.csPos = ds.projItemGlobToSys(ccore->cs, ccore->getGlobStart());
		ds.csSize = ds.projItemGlobToSys(ccore->cs, ccore->getGlobSize());
		if (ds.csSize.x == 0)
		{
			ds.csSize = BackPoint(300, 300);
		}
	}

	virtual void onClick(const GuiDisplaySystem&, BackPoint)
	{ }

	virtual int getSysId()
	{
		return copiedId;
	}
	virtual const char* getStrId()
	{
		return strId.c_str();
	}

	virtual ~IGuiLayer()
	{ }
};


export class LayerFactory : public CoreLayerFactory
{
private:
	static FunctionGuiHolder<IGuiLayer> guiLayersCreators;

public:
	static IGuiLayer* CreateGuiLayer(int id, ILayer* core = nullptr)
	{
		auto it = guiLayersCreators.find(id);
		if (it != guiLayersCreators.end())
			return it->second(core);
		else
			return nullptr;
	}

	static IGuiLayer* CreateGuiLayer(ILayer* core)
	{
		assert(core != nullptr);
		auto it = guiLayersCreators.find(core->getFactoryId());
		if (it != guiLayersCreators.end())
			return it->second(core);
		else
			return nullptr;
	}

	template <typename IGui, typename ICore>
	static void RegisterFactory(int id)
	{
		coreLayersCreators[id] = []() { return new ICore(); };
		guiLayersCreators[id] = [](ILayer* core) { return new IGui(static_cast<ICore*>(core)); };
	}
};

CoreLayerFactory::FunctionGuiHolder<IGuiLayer> LayerFactory::guiLayersCreators;

// Contexr

class ILayerWorker
{
public:
	InOutLayer iol;
	LayersList<IGuiLayer> layers;

	//LayersVals(GuiBackend& back) : backend(back)
	//{ }

	InOutLayer* getIoLayer()
	{
		return &iol;
	}

	template<typename T>
	T* getCastCurrentLayer()
	{
		IGuiLayer* l = getCurrentLayer();
		if (l)
			return dynamic_cast<T*>(l);
		else
			return nullptr;
	}

	IGuiLayer* getCurrentLayer()
	{
		return iol.in < 0 ? nullptr : layers.at(iol.in);
	}

	IGuiLayer* getTempLayer()
	{
		return iol.out < 0 ? nullptr : layers.at(iol.out);
	}

	void update()
	{
		if (iol.out == -1)
			return;
	}

	template<typename T>
	T* addLayer(const BackString& name)
	{
		auto t = layers.add<T>();
		settup(t, name);
		return t;
	}

	template<typename TGui, typename TData>
	TGui* addLayer(const BackString& name, TData* core)
	{
		if (core == nullptr)
			return nullptr;

		auto t = layers.add<TGui>(core);
		settup(t, name);
		return t;
	}

	template<typename TGui, typename TData>
	TGui* setLayer(const BackString& name, TData* core)
	{
		if (core == nullptr)
			return nullptr;

		auto ptr = new TGui(core);
		if (layers.set(core->id, ptr))
		{
			settup(ptr, name);
			return nullptr;
		}
		delete ptr;

		return addLayer<TGui, TData>(name, core);
	}

	void settup(IGuiLayer* layer, const BackString& name)
	{
		if (iol.in == -1)
		{
			iol.in = iol.out = layer->getSysId();
		}
		else
			iol.out = layer->getSysId();

		layer->setName(name, true);
		layer->toGuiData();
	}

	void setLayers(const RetLayers& rlayers, const BackString& name)
	{
		for (auto& lay : rlayers)
		{
			auto ptr = LayerFactory::CreateGuiLayer(lay.core);
			if (!layers.set(lay.core->id, ptr))
			{
				layers.addMove(ptr);
			}
			settup(ptr, lay.getName(name));
		}
	}

	void loadLayers()
	{
		layers.clear();
		for (auto& coreLay : proj->layers)
		{
			auto ptr = LayerFactory::CreateGuiLayer(coreLay.get());
			layers.addMove(ptr);
			settup(ptr, coreLay->name);
		}
	}

	void draw(const GuiDisplaySystem& ds)
	{
		uint i = 0;
		for (auto& lay : layers)
		{
			// ImGui::SetNextWindowPos(pos);
			// ImGui::SetNextWindowSize(size);
			// ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushID(i);
			lay->draw(ds);
			ImGui::PopID();
			++i;
		}

		//pos.x += drawSize.x;
		//ImGui::SetNextWindowPos(pos);
		//ImGui::SetNextWindowSize(nextSize);
		//ImGui::SetNextWindowViewport(viewport->ID);
	}

	virtual void drawToolbox()
	{
		auto t = getCurrentLayer();
		if (t)
			t->drawToolbox(*this);
	}

	void drawOverlap(const GuiDisplaySystem& ds)
	{
		auto t = getCurrentLayer();
		if (t)
			t->drawOverlap(ds);
	}

	void onClick(const GuiDisplaySystem& ds, BackPoint click)
	{
		if (iol.in == -1)
			return;
		auto layer = layers.at(iol.in);
		if (layer)
			layer->onClick(ds, click);
	}
};



export template<class T>
class GuiLayerData : public IGuiLayer
{
protected:
	T* data;

public:
	GuiImage icon;

	GuiLayerData(T* fromCore = nullptr)
	{
		if (fromCore == nullptr)
			data = proj->addLayerData<T>();
		else
			data = fromCore;

		copiedId = data->id;
	}

	virtual void toGuiData()
	{
		copiedId = data->getSysId();
		strId = data->name + intToStr(copiedId);
	}

	T* getData()
	{
		return data;
	}

	ILayer* getCore()
	{
		return data;
	}

	virtual GuiImage* getIcon()
	{
		return &icon;
	}

	virtual const char* getName() const
	{
		return data->name.c_str();
	}

	const LayerProvider& getProvider() const
	{
		return data->prov;
	}

	virtual void drawToolboxInner(ILayerWorker&)
	{ }

	virtual void drawToolbox(ILayerWorker& context)
	{
		if (ImGui::Begin("Toolbox"))
		{
			drawToolboxInner(context);
		}
		ImGui::End();
	}


	void setName(const BackString& name, bool updateOnlyEmpty = false)
	{
		if (data->name.length() != 0 && updateOnlyEmpty)
			return;

		data->name = name;
		strId = name + intToStr(data->id);
	}

	virtual ~GuiLayerData()
	{ }
};


export template<class IM, class T>
class ITiledRasterGuiLayer : public GuiLayerData<T>
{
public:
	IM main;
	GuiTilePreview tilePrview;
	int newTileSize;
	int newOffsetSize;

	ITiledRasterGuiLayer(T* fromCore) : GuiLayerData<T>(fromCore)
	{ }

	virtual ~ITiledRasterGuiLayer()
	{ }

	virtual void toGuiData()
	{
		GuiLayerData<T>::toGuiData();

		newTileSize = getTileSize() / 10;
		newOffsetSize = getOffsetSize() / 10;
	}

	virtual void draw(const GuiDisplaySystem& ds)
	{
		if (!IGuiLayer::visible)
			return;

		auto wpos = ds.getWinPos();
		auto& cs = GuiLayerData<T>::getCore()->cs;
		auto start = ds.projItemLocalToDisplay(cs, {0,0});
		auto end = ds.projItemLocalToDisplay(cs, main.getSize());

		main.drawImage(GuiLayerData<T>::getName(), wpos, ds.getDrawSize(), start, end);
	}

	inline int getTileSize() const
	{
		return GuiLayerData<T>::getProvider().tileSize;
	}
	inline int getOffsetSize() const
	{
		return GuiLayerData<T>::data->tileOffset;
	}
	inline int getImageMinSize() const
	{
		auto p = GuiLayerData<T>::getProvider();
 		return MIN(p.width, p.height);
	}

	inline ImVec2 getImageSize() const
	{
		auto p = GuiLayerData<T>::getProvider();
		return ImVec2(p.width, p.height);
	}

	void drawProperty()
	{
		ImGui::Text("Tile size");
		ImGui::SliderInt("##Tile size", &newTileSize, 1, getImageMinSize() / 10, "%d0");
		if (newTileSize + newOffsetSize > getImageMinSize() / 10)
			newOffsetSize = getImageMinSize() / 10 - newTileSize;

		ImGui::Text("Tile offset size");
		ImGui::SliderInt("##Offset size", &newOffsetSize, 0, getImageMinSize() / 10 - newTileSize, "%d0");

		ImGui::Separator();
		auto id = ImGui::FindWindowByName("ProcSetts")->ID;
		tilePrview.draw(id, newTileSize * 10, newOffsetSize * 10, getImageSize());
	}

	void applyPropertyChanges()
	{
		GuiLayerData<T>::data->prov.tileSize = newTileSize * 10;
		GuiLayerData<T>::data->tileOffset = newOffsetSize * 10;
	}

	SelectableKeyValues<int> imgSubImages;

	// Component
	SelectableKeyValues<bc::ComponentType> componentCB =
	{
		{bc::ComponentType::Component, "Компонента"},
		{bc::ComponentType::Hole, "Дыра"}
	};
	// ---

	// Proc Type
	SelectableKeyValues<bc::ProcType> procCB =
	{
		{bc::ProcType::f0t255, "От 0 до 255"},
		{bc::ProcType::f255t0, "От 255 до 0"},
		{bc::ProcType::Radius, "По расстоянию"},
		{bc::ProcType::invertf0, "Инвертировать"},
		{bc::ProcType::experiment, "Радар"},
		{bc::ProcType::ValueRadius, "Тру растояние"}
	};

	SelectableKeyValues<bc::ColorType> colorCB =
	{
		{bc::ColorType::native, "Как в изображении"},
		{bc::ColorType::gray, "Серый"},
		{bc::ColorType::rgb, "Цветной"},
	};

	SelectableKeyValues<int> alg =
	{
		{0, "Растровый"},
		{1, "Растр в точки"}
	};
	BarcodeProperies properties;
	GuiFilter filterInfo;

	void grabSets()
	{
		properties.barstruct.proctype = procCB.currentValue();
		properties.barstruct.coltype = colorCB.currentValue();
		properties.barstruct.comtype = componentCB.currentValue();
		properties.alg = alg.currentIndex;
	}

	void createBarcode(ILayerWorker& context)
	{
		grabSets();
		//RetLayers layerData = backend.createBarcode(layersVals.iol, properties, filterInfo.getFilter());
		RetLayers layerData = proj->createCacheBarcode(context.iol, properties, filterInfo.getFilter());
		context.setLayers(layerData, "barcode");
	}


	virtual void drawToolboxInner(ILayerWorker& context)
	{
		if (ImGui::Button("Построить баркод"))
		{
			auto subs = GuiLayerData<T>::data->getSubImageInfos();
			if (subs.size() != 0)
			{
				imgSubImages.clear();
				for (size_t i = 0; i < subs.size(); i++)
				{
					SubImgInf& sub = subs[i];
					BackString s = intToStr(sub.wid) + "x" + intToStr(sub.hei);
					imgSubImages.add(s, i);
				}
				imgSubImages.endAdding();
				imgSubImages.currentIndex = 0;
			}
			ImGui::OpenPopup("SelectMax");
		}

		if (ImGui::BeginPopupModal("SelectMax", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			alg.drawCombobox("Алгоритм");
			ImGui::Separator();

			if (alg.currentIndex == 0)
			{
				componentCB.drawCombobox("##Форма");
				procCB.drawCombobox("##Обработка");
				colorCB.drawCombobox("##Цвет");

			}
			else
			{
				ImGui::Checkbox("Use holes", &properties.alg1UseHoles);
				ImGui::Checkbox("ignore hight", &properties.alg1IgnoreHeight);
			}

			ImGui::Separator();
			ImGui::Text("Пороги отсеивания");
			filterInfo.draw();
			ImGui::Separator();

			if (imgSubImages.getSize() > 0)
			{
				imgSubImages.drawListBox("Размеры");
			}

			if (ImGui::Button("Запустить"))
			{
				GuiLayerData<T>::data->setSubImage(imgSubImages.currentIndex);

				ImGui::CloseCurrentPopup();
				createBarcode(context);
			}

			ImGui::SameLine();
			if (ImGui::Button("Отмена"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::Separator();
	}
};

template<class T>
class TiledRasterGuiLayer : public ITiledRasterGuiLayer<GuiDrawImage, T>
{
public:
	TiledRasterGuiLayer(T* fromCore) : ITiledRasterGuiLayer<GuiDrawImage, T>(fromCore)
	{ }

	virtual ~TiledRasterGuiLayer()
	{ }
};

export class RasterGuiLayer : public TiledRasterGuiLayer<RasterLayer>
{
public:

	RasterGuiLayer(RasterLayer* fromCore = nullptr) : TiledRasterGuiLayer<RasterLayer>(fromCore)
	{ }

	virtual void toGuiData()
	{
		TiledRasterGuiLayer::toGuiData();

		main.setImage(data->mat, false);
		icon.setImage(data->mat, 32, 32, true);
	}
};


export class RasterLineGuiLayer : public ITiledRasterGuiLayer<GuiDrawCloudPointClick, RasterLineLayer>
{
public:
	BackString debug;
	SimpleLine* selectedLine = nullptr;
	GuiFilter filtere;

	RasterLineGuiLayer(RasterLineLayer* fromCore = nullptr) : ITiledRasterGuiLayer(fromCore)
	{
	}

	virtual void toGuiData()
	{
		ITiledRasterGuiLayer::toGuiData();
		main.setImage(data->mat, false);
		icon.setImage(data->mat, 32, 32, true);
	}

	virtual void drawToolboxInner(ILayerWorker& context)
	{
		ITiledRasterGuiLayer::drawToolboxInner(context);

		if (ImGui::Button("Update"))
		{
			ImGui::OpenPopup("UpdateImage");
		}

		if (ImGui::BeginPopupModal("UpdateImage", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			filtere.draw();

			if (ImGui::Button("Update"))
			{
				selectedLine = NULL;
				main.unsetPoints();

				ImGui::CloseCurrentPopup();
				//auto rets = backend.processRaster(context.iol, filtere.getFilter());
				auto rets = proj->processCachedBarcode(context.iol, filtere.getFilter());

				context.setLayers(rets, "barcode");
				//commonValus.onAir = true;
				//commonValus.future = std::async(&GuiBackend::processRaster, std::ref(backend),
			}
			ImGui::SameLine();

			if (ImGui::Button("Close"))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::Separator();
	}

	virtual void draw(const GuiDisplaySystem& ds)
	{
		if (!visible)
			return;

		ITiledRasterGuiLayer::draw(ds);

		main.drawPoints(ds, getCore()->cs);
		drawLineInfoWin(ds);
	}

	virtual void onClick(const GuiDisplaySystem& ds, BackPoint pos)
	{
		ImVec2 wpos = ds.getWinPos();
		ImVec2 start = ds.projItemLocalToDisplay(data->cs, {0,0});
		ImVec2 posInDisplay = ds.projItemGlobToDisplay(data->cs, pos);
		if (ds.inDisplayRange(posInDisplay))
		{
			BackPixelPoint pix = data->cs.toLocal(toBP(posInDisplay - start));
			auto points = click((int)pix.x, (int)pix.y);
			setPoints(ds, points);
		}
	}


	void setPoints(const GuiDisplaySystem& ds, const bc::barvector* points)
	{
		main.setPoints(ds, data->cs, points);
	}


	bc::barvector* click(int x, int y)
	{
		if (data->clickResponser.size() == 0)
			return nullptr;

		x = main.getRealX(x);
		y = main.getRealY(y);


		if (x < 0 || x >= main.width)
			return NULL;

		if (y < 0 || y >= main.height)
			return NULL;

		//std::cout << x << " : " << y << std::endl;

		SimpleLine* line = data->clickResponser[y * main.width + x].get();
		if (line)
		{
			if (selectedLine == line && line->parent)
				line = line->parent.get();

			selectedLine = line;
			return &(selectedLine->matr);
		}

		return nullptr;
	}

	//void unsetPoints()
	//{
	//	centerVals.clickHandler.points = nullptr;
	//	centerVals.processImage.clicked = false;
	//}


	SimpleLine* moveToParenr()
	{
		return selectedLine = selectedLine->parent.get();
	}

	void drawLineInfoWin(const GuiDisplaySystem& ds)
	{
		SimpleLine* line = selectedLine;
		if (!line)
		{
			return;
		}

		if (!ImGui::Begin("Propertis"))
		{
			ImGui::End();
			return;
		}

		if (line->depth > 0)
		{
			ImGui::BeginDisabled(line->parent == nullptr);
			if (ImGui::Selectable("Parent"))
			{
				line = moveToParenr();
				setPoints(ds, &line->matr);
			}
			ImGui::EndDisabled();

			ImGui::Separator();
			BackString s = line->start.text<BackString, toStdStr>();
			s = "Start: " + s;
			ImGui::Text(s.c_str());
			s = line->end.text<BackString, toStdStr>();
			s = "End: " + s;
			ImGui::Text(s.c_str());

			ImGui::Text("Depth %d", line->depth);
			ImGui::Text("Matr size %d", line->matrSrcSize);
		}
		else
		{
			ImGui::Text("The root has been reached");
		}

		ImGui::End();
	}
private:
};


export class RasterFromDiskGuiLayer : public TiledRasterGuiLayer<RasterFromDiskLayer>
{
public:
	HeimapOverlap heimap;
	TilemapOverlap tilemap;

	bool drawHeimap;

	RasterFromDiskGuiLayer(RasterFromDiskLayer* fromCore = nullptr) : TiledRasterGuiLayer(fromCore)
	{
		drawHeimap = false;
		heimap.enable = false;
	}

	virtual void toGuiData()
	{
		TiledRasterGuiLayer::toGuiData();

		auto i = *data->getCachedImage();
		main.setImage(i, false);
		icon.setImage(i, 32, 32, true);
	}

	virtual void drawToolboxInner(ILayerWorker& context)
	{
		ITiledRasterGuiLayer::drawToolboxInner(context);

		if (ImGui::Button("Activation"))
		{
			auto rets = proj->exeFilter(context.iol, 0);
			context.setLayers(rets, "filter");
		}

		//ImGui::SameLine();
		//ImGui::Checkbox("Переключить вид", &heimap.enable);
		//if (heimap.enable && !heimap.isInit())
		//{
		//	heimap.init(main);
		//}
		ImGui::Separator();
	}

	virtual void drawOverlap(const GuiDisplaySystem& ds)
	{
		if (drawHeimap)
		{
			//heimap.draw(ds);
		}
		else
		{
			// tilemap.init(&main, &getProvider());
			// tilemap.draw(main.displaysBegin, main.displaySize);
		}
	}

	std::vector<SubImgInf> getSubImageInfos()
	{
		return data->getSubImageInfos();
	}
};


// Vector layer

export class VectorGuiLayer : public GuiLayerData<VectorLayer>
{
	ImVec4 propColor;
public:

	VectorGuiLayer(VectorLayer* fromCore) : GuiLayerData<VectorLayer>(fromCore)
	{ }

	virtual ~VectorGuiLayer()
	{ }

	virtual void toGuiData()
	{
		GuiLayerData<VectorLayer>::toGuiData();
		propColor.x = static_cast<float>(data->color.r) / 255.f;
		propColor.y = static_cast<float>(data->color.g) / 255.f;
		propColor.z = static_cast<float>(data->color.b) / 255.f;
		//for (auto& d : data->primitives.draws)
		//{
		//	points.push_back(ImVec2(d.x, d.y));
		//}
	}

	virtual void draw(const GuiDisplaySystem& ds)
	{
		if (!IGuiLayer::visible)
			return;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

		ImGui::SetCursorPos(ds.getDrawPos());
		if (!ImGui::BeginChild(data->name.c_str(), ds.getDrawSize(), false, window_flags))
		{
			ImGui::EndChild();
			return;
		}


		switch (data->vecType)
		{
		case VectorLayer::VecType::points:
			drawPoints(ds);
			break;
		case VectorLayer::VecType::polygons:
			drawPolygons(ds);
			break;
		default:
			break;
		}

		ImGui::EndChild();
	}


	void drawPoints(const GuiDisplaySystem& ds)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImDrawList* list = window->DrawList;

		ImColor bigColor(128, 0, 255);
		ImColor midColor(220, 200, 0);
		float markerSize = 2;//MAX(1, par->displaySize.x / par->width);


		auto& d = data->primitives;
		if (d.size() == 0)
			return;

		BackColor cscol = data->color;
		ImColor col(cscol.r, cscol.g, cscol.b);

		auto& cs = data->cs;

		BackPoint itemSt = ds.getSysToItemStartPos(cs);
		BackPoint ed = ds.getSysToItemEndPos(cs);

		auto& points = d[0].points;
		for (auto& p : points)
		{
			if (GuiDisplaySystem::inRange(itemSt, ed, p))
			{
				ImVec2 pi = ds.projItemGlobToDisplay(cs, p);
				list->AddCircleFilled(pi, 3, col);
			}
		}

		//ImColor bigColor(128, 0, 255);
		//ImColor midColor(220, 200, 0);
		//float markerSize = 2;//MAX(1, par->displaySize.x / par->width);

		//for (const auto& p : points)
		//{
		//	// TL is a Begin()
		//	ItemVec2 pi = p - ds.pos;// (par->toDisplayX(p.getX()), par->toDisplayY(p.getY()));
		//	//data->cs.

		//	if (pi.x < csreenStar.x || pi.y < csreenStar.y)
		//		continue;
		//	if (pi.x > csreenEnd.x || pi.y > csreenEnd.y)
		//		continue;

		//	auto p = ds.projItemGlobToLoc(cs, pi);
		//	pi = offset + ImVec2(p.x, p.y);

		//	// Center pixel for big images
		//	list->AddCircleFilled(pi, 1.5 * markerSize, bigColor);
		//	list->AddCircleFilled(pi, 0.8 * markerSize, midColor);
		//}
	}

	void drawPolygons(const GuiDisplaySystem& ds)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImDrawList* list = window->DrawList;
		ImVec2 offset = window->Pos;


		CSBinding& dsc = getCore()->cs;
		BackPoint start = ds.getSysToItemStartPos(dsc);
		BackPoint end = ds.getSysToItemEndPos(dsc);
		// auto start = ds.getDisplayStartPos();
		// auto end = ds.getDisplayEndPos();

		BackColor cscol = data->color;
		for (const auto& d : data->primitives)
		{
			const auto& points = d.points;
			// cscol = d.color;
			ImColor col(cscol.r, cscol.g, cscol.b);

			if (points.size() < 3)
				continue;

			int added = 0;
			bool prevIsOut = false;
			ImVec2 prev;
			for (const BackPoint& p : points)
			{
				// if (!GuiDisplaySystem::inRange(start, end, p))
				// {
				// 	if (prevIsOut)
				// 		continue;

				// 	prevIsOut = true;
				// }
				// else if (prevIsOut)
				// {
				// 	++added;
				// 	list->PathLineTo(prev);
				// 	prevIsOut = false;
				// }

				if (!GuiDisplaySystem::inRange(start, end, p))
				{
					continue;
				}

				ImVec2 pi = ds.projItemGlobToDisplay(dsc, p) + offset;
				prev = pi;

				++added;
				list->PathLineTo(pi);
			}

			if (added >= 3)
			{
				ImVec2 pi = ds.projItemGlobToDisplay(dsc, points[0]) + offset; // First
				list->PathFillConvex(col);
			}

			list->PathClear();
		}
	}


	static void getRect(const std::vector<BackPoint>& points, ImVec2& itemSt, ImVec2& end)
	{
		itemSt = end = toIV(points[0]);
		for (const BackPoint& p : points)
		{
			if (p.x < itemSt.x)
			{
				itemSt.x = p.x;
			}

			if (p.y < itemSt.y)
			{
				itemSt.y = p.y;
			}

			if (p.x > end.x)
			{
				end.x = p.x;
			}

			if (p.y < end.y)
			{
				end.y = p.y;
			}
		}
	}

	void drawProperty()
	{
		ImGui::ColorPicker4("Color", (float*)&propColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs);
	}

	void applyPropertyChanges()
	{
		data->color.r = std::min(static_cast<int>(propColor.x * 255), 255);
		data->color.g = std::min(static_cast<int>(propColor.y * 255), 255);
		data->color.b = std::min(static_cast<int>(propColor.z * 255), 255);
	}
};

export class LayersVals : public ILayerWorker
{
public:

	IRasterLayer* getCurrentRasterCore()
	{
		return iol.in < 0 ? nullptr : dynamic_cast<IRasterLayer*>(layers.at(iol.in)->getCore());
	}

	// RasterFromDiskGuiLayer* addImageFromDiskLayer()
	// {
	// 	// layers.clear();
	// 	// proj->layers.clear();
	// 	RasterFromDiskGuiLayer* val = layers.add<RasterFromDiskGuiLayer>();
	// 	settup(val, "From disk");
	// 	iol.out = -1;
	// 	return val;
	// }

	void drawLayersWindow()
	{
		if (!ImGui::Begin("Layers"))
		{
			ImGui::End();
			return;
		}

		ImVec2 pos = ImGui::GetWindowPos();

		auto winsize = ImGui::GetWindowSize();
		winsize.y -= 50;

		bool temporaly = false;
		int displayRadioId = iol.out;
		if (layers.size() > 0 && iol.out == -1)
			displayRadioId = (*layers.begin())->getSysId();

		float selHei = 40;
		ImVec2 iconSize(40, selHei / 2);

		bool catchNext = false;
		int toMove[2]{ -1, -1 };
		int prevId = -1;
		int delId = -1;

		if (ImGui::BeginListBox("##LayersList", winsize))
		{
			uint j = 0;
			for (auto& lay : layers)
			{
				bool fistLayer = j == 0;
				bool lastLayer = j == layers.size() - 1;
				//if (lay->getSysId()s == iol.out)
				//{
				//	ImFont* italicFont = ImGui::GetIO().Fonts->Fonts[1];  // Assuming the second font in the ImFontAtlas is italic
				//	ImGui::PushFont(italicFont);
				//}
				ImGui::PushID(j);

				auto curID = lay->getSysId();
				if (catchNext)
				{
					toMove[1] = curID;
					catchNext = false;
				}

				auto& icon = *lay->getIcon();

				bool curRadio = (displayRadioId == lay->getSysId());
				temporaly = ImGui::RadioButton("##Work", curRadio);//, curRadio);

				ImGui::SameLine();
				ImGui::Checkbox("##visible", &lay->visible);

				ImGui::SameLine();
				ImGui::Image((void*)(intptr_t)icon.getTextureId(), ImVec2(selHei, selHei));

				ImGui::SameLine();
				auto posBef = ImGui::GetCursorPos();
				ImGui::BeginDisabled(fistLayer);
				if (ImGui::Button(ICON_FA_ANGLE_UP "", iconSize))
				{
					toMove[0] = prevId;
					toMove[1] = curID;
				}
				ImGui::EndDisabled();

				ImGui::SetCursorPos({posBef.x, posBef.y + iconSize.y});
				ImGui::BeginDisabled(lastLayer);
				if (ImGui::Button(ICON_FA_ANGLE_DOWN "", iconSize))
				{
					toMove[0] = curID;
					catchNext = true;
				}
				ImGui::EndDisabled();

				ImGui::SetCursorPos({posBef.x + iconSize.x, posBef.y});
				if (ImGui::Button(ICON_FA_TRASH "", ImVec2(selHei, selHei)))
				{
					delId = curID;
					lay->getCore()->release(proj->getMeta());
				}

				ImGui::SameLine();
				bool seled = ImGui::Selectable(lay->getName(), curID == iol.in, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(winsize.x - 50, selHei));
				prevId = curID;

				if (seled)
				{
					iol.in = lay->getSysId();
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						lay->lockAtThis();
					}
				}
				if (temporaly)
				{
					if (j == 0)
						iol.out = -1;
					else
						iol.out = lay->getSysId();

					displayRadioId = lay->getSysId();
					temporaly = false;
				}

				//if (j == iol.out)
				//{
				//	ImGui::PopFont();
				//}
				// ImVec4 color(1.0f, 0.0f, 0.0f, 1.0f); // RGBA color (red in this case)
				ImGui::PopID();
				++j;
			}
			ImGui::EndListBox();
		}

		if (delId != -1)
		{
			layers.remove(delId);
			proj->removeLayer(delId);
		}

		//ImGui::EndGroup();

		ImGui::End();
		if (toMove[0] != -1 && toMove[1] != -1)
		{
			proj->layers.move(toMove[0], toMove[1]);
			layers.move(toMove[0], toMove[1]);
		}
	}
};
