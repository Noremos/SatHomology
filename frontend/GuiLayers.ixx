module;
#include "GuiCommon.h"

#include <numeric>

export module LayersGui;

import RasterLayers;
import ProjectModule;
import BarcodeModule;
import GuiWidgets;
import GuiOverlap;
import IOCore;
import VectorLayers;
import CSBind;
import Platform;
//import LuaStates;

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

	void lockAtThis(ImVec2 realSize)
	{
		auto* ccore = getCore();
		DisplaySystem& ds = proj->getDisplay();

		BackPoint BPRealSize = toBP(realSize);

		BackPoint start = ds.projItemGlobToSys(ccore->cs, ccore->getNormGlobStart());
		BackPoint IT = ds.projItemGlobToSys(ccore->cs, ccore->getNormGlobSize());

		BackPoint IR = IT;
		ResizeImage(IR, BPRealSize);

		ds.csScale = IR.x / IT.x;

		BackPoint winSizeInSys = ds.toSysGlobRelative(BPRealSize);
		BackPoint itemSize = ds.toSysGlobRelative(IR);
		ds.csPos = start - (winSizeInSys - itemSize) / 2;
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

export class ILayerWorker
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

	ImVec2 lastRealSize;

	void draw(const GuiDisplaySystem& ds)
	{
		lastRealSize = toIV(ds.drawSize);

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
		projSet.setup(data->cs);
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

	ProjectionSettings projSet;
	virtual void drawProperty()
	{
		projSet.draw();
	}
	virtual void applyPropertyChanges()
	{
		projSet.setup(data->cs);
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



// Vector layer

export class VectorGuiLayer : public GuiLayerData<VectorLayer>
{
	ImVec4 propColor;

	enum ChangeState
	{
		none,
		add,
		edit,
		remove
	} checkState = ChangeState::none;

	template<class E, E curval>
	class CheckSate
	{
		bool state = false;
		E& valRef;

	public:
		CheckSate(E& valRef) : valRef(valRef)
		{ }

		void draw(const char* name)
		{
			ImGui::Checkbox(name, &state);
			if (state)
			{
				valRef = curval;
			}
		}
	};

	CheckSate<ChangeState, ChangeState::add> addCB;
	CheckSate<ChangeState, ChangeState::edit> editCB;
	CheckSate<ChangeState, ChangeState::remove> removeCB;
	bool isInChangeMode() const
	{
		return checkState != ChangeState::none;
	}

	std::vector<size_t> orders;

public:

	VectorGuiLayer(VectorLayer* fromCore) : GuiLayerData<VectorLayer>(fromCore),
		addCB(checkState), editCB(checkState), removeCB(checkState)
	{ }

	virtual ~VectorGuiLayer()
	{ }

	virtual void toGuiData()
	{
		GuiLayerData<VectorLayer>::toGuiData();
		propColor.x = static_cast<float>(data->color.r) / 255.f;
		propColor.y = static_cast<float>(data->color.g) / 255.f;
		propColor.z = static_cast<float>(data->color.b) / 255.f;

		orders.resize(data->primitives.size());
		std::iota(orders.begin(), orders.end(), 0);

		if (data->vecType == VectorLayer::VecType::polygons)
		{
			std::sort(orders.begin(), orders.end(), [&](size_t a, size_t b) {
				return data->primitives[a]->points.size() < data->primitives[b]->points.size();
			});
		}
	}

	virtual void draw(const GuiDisplaySystem& ds)
	{
		if (!IGuiLayer::visible)
			return;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings
		 | ImGuiWindowFlags_NoScrollWithMouse;


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
		case VectorLayer::VecType::circles:
			drawCircles(ds);
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


		auto& prims = data->primitives;
		if (prims.size() == 0)
			return;

		BackColor cscol = data->color;
		ImColor col(cscol.r, cscol.g, cscol.b);
		ImColor cursorColor(255 - cscol.r, 255 - cscol.g, 255 - cscol.b);

		auto& cs = data->cs;

		BackPoint itemSt = ds.getSysToItemStartPos(cs);
		BackPoint ed = ds.getSysToItemEndPos(cs);

		for (DrawPrimitive* d : prims)
		{
			auto& points = d->points;

			BackPoint p = points[0];
			if (GuiDisplaySystem::inRange(itemSt, ed, p))
			{
				ImVec2 pi = ds.projItemGlobToDisplay(cs, p);
				if (isInChangeMode() && d->isNearPoint(ds.cursorPos))
				{
					list->AddCircleFilled(pi, 3, cursorColor);
				}
				else
					list->AddCircleFilled(pi, 3, col);
			}
		}
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

		ImU32 colbl = ImColor(255, 255, 255);

		BackColor cscol = data->color;
		for (size_t& io : orders)
		{
			const DrawPrimitive* d = data->primitives[io];

			const std::vector<BackPoint>& points = d->points;
			cscol = d->color;
			// ImU32 col = ImColor(cscol.r, cscol.g, cscol.b, 200);
			ImU32 col = ImColor(cscol.r, cscol.g, cscol.b, 255);
			ImU32 colRev = ImColor(255 - cscol.r, 255 - cscol.g, 255 - cscol.b);

			// if (d->set)

			if (points.size() < 3)
				continue;

			BackPoint point = ds.cursorPos;
			ImU32 cursorColor = ImColor(255 - cscol.r, 255 - cscol.g, 255 - cscol.b);

			int visible = 0;
			bool inside = false;
			std::vector<ImVec2> displayPoints;
			int n = points.size();
			for (int i = 0, j = n - 1; i < n; j = i++)
			{
				const BackPoint& p = points[i];
				const BackPoint& pj = points[j];

				if (((p.y > point.y) != (pj.y > point.y)) &&
					(point.x < (pj.x - p.x) * (point.y - p.y) / (pj.y - p.y) + p.x))
				{
					inside = !inside;
				}

				if (GuiDisplaySystem::inRange(start, end, p))
				{
					++visible;
				}

				ImVec2 pi = ds.projItemGlobToDisplay(dsc, p) + offset;
				displayPoints.push_back(pi);
				// if (inside) // Spot on cursor
				// 	list->AddCircleFilled(pi, 3, cursorColor);
			}

			if (visible >= 1)
			{
				displayPoints.push_back(displayPoints[0]);

				ImVec2 pi = ds.projItemGlobToDisplay(dsc, points[0]) + offset; // First
				// list->AddConvexPolyFilled(displayPoints.data(), displayPoints.size(), col);
				// list->AddPolyline(displayPoints.data(), displayPoints.size(), colbl, 0, 2.0);
				list->AddPolyline(displayPoints.data(), displayPoints.size(), col, 0, 3.0);

				if (inside)
				{
					list->AddPolyline(displayPoints.data(), displayPoints.size(), colRev, 0, 3.0);
				}
			}
			displayPoints.clear();
		}
	}

	void drawCircles(const GuiDisplaySystem& ds)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImDrawList* list = window->DrawList;
		ImVec2 offset = window->Pos;


		CSBinding& dsc = getCore()->cs;
		const BackPoint start = ds.getSysToItemStartPos(dsc);
		const BackPoint end = ds.getSysToItemEndPos(dsc);

		BackColor cscol = data->color;
		for (const DrawPrimitive* d : data->primitives)
		{
			const std::vector<BackPoint>& points = d->points;
			if (points.size() == 0)
				return;

			assert(points.size() == 2);
			{
				BackPoint rad(points[1].x - points[0].x, points[1].y - points[1].y);

				std::vector<BackPoint> inCheck;
				inCheck.push_back(points[0]);
				inCheck.push_back(BackPoint(points[0].x - rad.x, points[0].y - rad.y));
				inCheck.push_back(BackPoint(points[0].x - rad.x, points[0].y + rad.y));
				inCheck.push_back(BackPoint(points[0].x + rad.x, points[0].y - rad.y));
				inCheck.push_back(points[1]);

				bool inDraw = false;
				for (int i = 0; i < inCheck.size(); ++i)
				{
					const BackPoint& p = inCheck[i];
					if (GuiDisplaySystem::inRange(start, end, p))
					{
						inDraw = true;
					}
				}

				if (!inDraw)
					continue;
			}

			// ImU32 col = ImColor(cscol.r, cscol.g, cscol.b, 200);
			ImU32 col = ImColor(cscol.r, cscol.g, cscol.b, 255);
			ImVec2 pi = ds.projItemGlobToDisplay(dsc, points[0]) + offset; // First
			ImVec2 radDisplay = ds.projItemGlobToDisplay(dsc, points[1]) + offset; // Radius

			const float rX = radDisplay.x - pi.x;
			const float rY = radDisplay.y - pi.y;
			list->AddCircle(pi, rX > rY ? rX : rY, col, 0, 5);
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

	void drawToolboxInner(ILayerWorker& context)
	{
		if (ImGui::Button("Выгрузить слой"))
		{
			BackPathStr path = getSavePath({ "geojson", "*.geojson",
											"json", "*.json" });
			data->savePolygonsAsGeojson(path);
		}

		// Add ploygon;
		// Add line
		// Move point
		// Move line
		// Drop
		// export
		// import


		addCB.draw("Режим добавления");
		editCB.draw("Режим изменения");
		removeCB.draw("Режим удаления");

		switch (data->vecType)
		{
		case VectorLayer::VecType::points:
			drawPointsToolbox(context);
			break;
		case VectorLayer::VecType::polygons:
			drawPolygonToolbox(context);
			break;
		default:
			break;
		}

	}


	void drawPointsToolbox(ILayerWorker& context)
	{
	}

	void drawPolygonToolbox(ILayerWorker& context)
	{}

	virtual void drawProperty()
	{
		GuiLayerData<VectorLayer>::drawProperty();
		ImGui::Separator();

		ImGui::ColorPicker4("Color", (float*)&propColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs);
	}

	virtual void applyPropertyChanges()
	{
		GuiLayerData<VectorLayer>::applyPropertyChanges();
		ImGui::Separator();

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
						lay->lockAtThis(lastRealSize);
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
