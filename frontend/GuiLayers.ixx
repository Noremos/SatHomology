module;
#include "DrawCommon.h"

#include <numeric>
#include <algorithm>
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "Usings.h"

export module GuiLayers;

import LayersCore;
import RasterLayers;
import ProjectModule;
// import BarcodeModule;
import GuiWidgets;
import GuiOverlap;
import VectorLayers;
import CSBind;
import Platform;
import DrawUtils;
import BackBind;
//import LuaStates;

//import FrontendBind;

export class LayersVals;
export class ILayerWorker;

Project* proj = Project::getProject();
//GuiBackend backend;

export class IGuiLayer
{
protected:
	BackString strId;
	int copiedId = -1; // We need to cpy Id to avoid cases when core was destroid
public:
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

	ImVec2 lastRealSize;

	void draw(const GuiDisplaySystem& ds)
	{
		lastRealSize = toIV(ds.drawSize);

		buint i = 0;
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
		strId = data->name;
		strId += intToStr(copiedId);
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
		if (ImGui::Begin("Инструмаенты слоя"))
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
		strId = name;
		strId += intToStr(data->id);
	}

	virtual ~GuiLayerData()
	{ }
};



// Vector layer
export template<class T>
class VectorBaseLayer : public GuiLayerData<T>
{
	using Base = GuiLayerData<T>;
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
	VectorBaseLayer(T* fromCore) : GuiLayerData<T>(fromCore),
		addCB(checkState), editCB(checkState), removeCB(checkState)
	{ }

	virtual ~VectorBaseLayer()
	{ }

	virtual void toGuiData()
	{
		Base::toGuiData();
		propColor.x = static_cast<float>(Base::data->color.r) / 255.f;
		propColor.y = static_cast<float>(Base::data->color.g) / 255.f;
		propColor.z = static_cast<float>(Base::data->color.b) / 255.f;

		orders.resize(Base::data->primitives.size());
		std::iota(orders.begin(), orders.end(), 0);

		if (Base::data->vecType == VectorLayer::VecType::polygons)
		{
			std::sort(orders.begin(), orders.end(), [&](size_t a, size_t b) {
				return Base::data->primitives[a]->points.size() < Base::data->primitives[b]->points.size();
			});
		}
	}

	virtual void draw(const GuiDisplaySystem& ds)
	{
		if (!Base::data->visible)
			return;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings
		 | ImGuiWindowFlags_NoScrollWithMouse;


		ImGui::SetCursorPos(ds.getDrawPos());
		if (!ImGui::BeginChild(Base::data->name.c_str(), ds.getDrawSize(), false, window_flags))
		{
			ImGui::EndChild();
			return;
		}


		switch (Base::data->vecType)
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

		auto& prims = Base::data->primitives;
		if (prims.size() == 0)
			return;

		BackColor cscol = Base::data->color;
		ImColor col(cscol.r, cscol.g, cscol.b);
		ImColor cursorColor(255 - cscol.r, 255 - cscol.g, 255 - cscol.b);

		auto& cs = Base::data->cs;

		BackPoint itemSt = ds.getSysToItemStartPos(cs);
		BackPoint ed = ds.getSysToItemEndPos(cs);

		int i = 0;
		for (DrawPrimitive* d : prims)
		{
			auto& points = d->points;
			float tickness = spotId == i ? 3 : 10;

			BackPoint p = points[0];
			if (GuiDisplaySystem::inRange(itemSt, ed, p))
			{
				ImVec2 pi = ds.projItemGlobToDisplay(cs, p);
				if (isInChangeMode() && d->isNearPoint(ds.cursorPos))
				{
					list->AddCircleFilled(pi, tickness, cursorColor);
				}
				else
					list->AddCircleFilled(pi, tickness, col);
			}

			++i;
		}
	}

	//bool areIntersecting(
	//	float v1x1, float v1y1, float v1x2, float v1y2,
	//	float v2x1, float v2y1, float v2x2, float v2y2)
	//{
	//	float d1, d2;
	//	float a1, a2, b1, b2, c1, c2;

	//	// Convert vector 1 to a line (line 1) of infinite length.
	//	// We want the line in linear equation standard form: A*x + B*y + C = 0
	//	// See: http://en.wikipedia.org/wiki/Linear_equation
	//	a1 = v1y2 - v1y1;
	//	b1 = v1x1 - v1x2;
	//	c1 = (v1x2 * v1y1) - (v1x1 * v1y2);

	//	// Every point (x,y), that solves the equation above, is on the line,
	//	// every point that does not solve it, is not. The equation will have a
	//	// positive result if it is on one side of the line and a negative one
	//	// if is on the other side of it. We insert (x1,y1) and (x2,y2) of vector
	//	// 2 into the equation above.
	//	d1 = (a1 * v2x1) + (b1 * v2y1) + c1;
	//	d2 = (a1 * v2x2) + (b1 * v2y2) + c1;

	//	// If d1 and d2 both have the same sign, they are both on the same side
	//	// of our line 1 and in that case no intersection is possible. Careful,
	//	// 0 is a special case, that's why we don't test ">=" and "<=",
	//	// but "<" and ">".
	//	if (d1 > 0 && d2 > 0) return false;
	//	if (d1 < 0 && d2 < 0) return false;

	//	// The fact that vector 2 intersected the infinite line 1 above doesn't
	//	// mean it also intersects the vector 1. Vector 1 is only a subset of that
	//	// infinite line 1, so it may have intersected that line before the vector
	//	// started or after it ended. To know for sure, we have to repeat the
	//	// the same test the other way round. We start by calculating the
	//	// infinite line 2 in linear equation standard form.
	//	a2 = v2y2 - v2y1;
	//	b2 = v2x1 - v2x2;
	//	c2 = (v2x2 * v2y1) - (v2x1 * v2y2);

	//	// Calculate d1 and d2 again, this time using points of vector 1.
	//	d1 = (a2 * v1x1) + (b2 * v1y1) + c2;
	//	d2 = (a2 * v1x2) + (b2 * v1y2) + c2;

	//	// Again, if both have the same sign (and neither one is 0),
	//	// no intersection is possible.
	//	if (d1 > 0 && d2 > 0) return false;
	//	if (d1 < 0 && d2 < 0) return false;

	//	// If we get here, only two possibilities are left. Either the two
	//	// vectors intersect in exactly one point or they are collinear, which
	//	// means they intersect in any number of points from zero to infinite.
	//	if ((a1 * b2) - (a2 * b1) == 0.0f) return true;

	//	// If they are not collinear, they must intersect in exactly one point.
	//	return false;
	//}

	void drawPolygon(const DrawPrimitive* d, const GuiDisplaySystem& ds, float tick = 3.0)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImDrawList* list = window->DrawList;
		ImVec2 offset = window->Pos;

		const CSBinding& dsc = Base::data->cs;
		const BackPoint start = ds.getSysToItemStartPos(dsc);
		const BackPoint end = ds.getSysToItemEndPos(dsc);

		const std::vector<BackPoint>& points = d->points;
		auto cscol = d->color;
		// ImU32 col = ImColor(cscol.r, cscol.g, cscol.b, 200);
		ImU32 col = ImColor(cscol.r, cscol.g, cscol.b, 255);
		ImU32 colRev = ImColor(255 - cscol.r, 255 - cscol.g, 255 - cscol.b);

		if (points.size() < 3)
			return;

		BackPoint point = ds.cursorPos;
		ImU32 cursorColor = ImColor(255 - cscol.r, 255 - cscol.g, 255 - cscol.b);

		int pointsVisible = 0;
		bool inside = false;
		std::vector<ImVec2> displayPoints;
		int n = (int)points.size();
		for (int i = 0, j = n - 1; i < n; j = i++)
		{
			const BackPoint& a = points[i];
			const BackPoint& c = points[j];

			if (((a.y > point.y) != (c.y > point.y)) &&
				(point.x < (c.x - a.x) * (point.y - a.y) / (c.y - a.y) + a.x))
			{
				inside = !inside;
			}


			if (GuiDisplaySystem::inRange(start, end, a))
			{
				++pointsVisible;
			}

			ImVec2 pi = ds.projItemGlobToDisplay(dsc, a) + offset;
			displayPoints.push_back(pi);
			 //if (inside) // Spot on cursor
			 //	list->AddCircleFilled(pi, 3, cursorColor);
		}

		if (pointsVisible >= 1)
		{
			displayPoints.push_back(displayPoints[0]);

			ImVec2 pi = ds.projItemGlobToDisplay(dsc, points[0]) + offset; // First
			// list->AddConvexPolyFilled(displayPoints.data(), displayPoints.size(), col);
			// list->AddPolyline(displayPoints.data(), displayPoints.size(), colbl, 0, 2.0);
			list->AddPolyline(displayPoints.data(), displayPoints.size(), col, 0, tick);

			if (inside)
			{
				list->AddPolyline(displayPoints.data(), displayPoints.size(), colRev, 0, tick * 1.3);
			}
		}
		displayPoints.clear();
	}

	int spotId = -1;
	void drawPolygons(const GuiDisplaySystem& ds)
	{
		// auto start = ds.getDisplayStartPos();
		// auto end = ds.getDisplayEndPos();

		ImU32 colbl = ImColor(255, 255, 255);

		BackColor cscol = Base::data->color;
		for (size_t& io : orders)
		{
			const DrawPrimitive* d = Base::data->primitives[io];
			drawPolygon(d, ds, io == spotId ? 10.0 : 3.0);
		}
	}

	void drawCircles(const GuiDisplaySystem& ds)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImDrawList* list = window->DrawList;
		ImVec2 offset = window->Pos;


		CSBinding& dsc = Base::data->cs;
		const BackPoint start = ds.getSysToItemStartPos(dsc);
		const BackPoint end = ds.getSysToItemEndPos(dsc);

		BackColor cscol = Base::data->color;
		for (const DrawPrimitive* d : Base::data->primitives)
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

	void drawToolboxInner(ILayerWorker& context)
	{
		if (ImGui::Button("Выгрузить слой"))
		{
			BackPathStr path = getSavePath({ "geojson", "*.geojson",
											"json", "*.json" });
			Base::data->savePolygonsAsGeojson(path);
		}

		// Add ploygon;
		// Add line
		// Move point
		// Move line
		// Drop
		// export
		// import


		//addCB.draw("Режим добавления");
		//editCB.draw("Режим изменения");
		//removeCB.draw("Режим удаления");

		//switch (Base::data->vecType)
		//{
		//case VectorLayer::VecType::points:
		//	drawPointsToolbox(context);
		//	break;
		//case VectorLayer::VecType::polygons:
		//	drawPolygonToolbox(context);
		//	break;
		//default:
		//	break;
		//}

	}


	void drawPointsToolbox(ILayerWorker& context)
	{
	}

	void drawPolygonToolbox(ILayerWorker& context)
	{}

	virtual void drawProperty()
	{
		Base::drawProperty();
		ImGui::Separator();

		ImGui::ColorPicker4("Color", (float*)&propColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs);
	}

	virtual void applyPropertyChanges()
	{
		Base::applyPropertyChanges();
		ImGui::Separator();

		Base::data->color.r = std::min(static_cast<int>(propColor.x * 255), 255);
		Base::data->color.g = std::min(static_cast<int>(propColor.y * 255), 255);
		Base::data->color.b = std::min(static_cast<int>(propColor.z * 255), 255);

		//if (applyForPrimitives)
		{
			for (DrawPrimitive* d : Base::data->primitives)
			{
				d->color = Base::data->color;
			}
		}
	}
};


class LayersVals : public ILayerWorker
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



	//void drawSubLayers(int& itemCounter, bool allowChange)
	//{
	//	bool catchNext = false, incrementCurrent = false, incrementWorkingLayer = false;
	//	int toMove[2]{ -1, -1 };
	//	int prevId = -1;
	//	int delId = -1;

	//	if (displayRadioId == -1)
	//		displayRadioId = 0;

	//	buint j = 0;
	//	for (auto& lay : layers)
	//	{
	//		bool fistLayer = j == 0;
	//		bool lastLayer = j == layers.size() - 1;
	//		//if (lay->getSysId()s == iol.out)
	//		//{
	//		//	ImFont* italicFont = ImGui::GetIO().Fonts->Fonts[1];  // Assuming the second font in the ImFontAtlas is italic
	//		//	ImGui::PushFont(italicFont);
	//		//}
	//		ImGui::PushID(itemCounter++);

	//		auto curID = lay->getSysId();
	//		if (catchNext)
	//		{
	//			toMove[1] = curID;
	//			catchNext = false;
	//		}

	//		if (incrementCurrent)
	//		{
	//			incrementCurrent = false;
	//			iol.in = curID;
	//		}

	//		if (incrementWorkingLayer)
	//		{
	//			incrementWorkingLayer = false;
	//			displayRadioId = curID;
	//		}

	//		auto& icon = *lay->getIcon();

	//		bool curRadio = (displayRadioId == lay->getSysId());
	//		updateWorkingImage = ImGui::RadioButton("##Work", curRadio);//, curRadio);

	//		ImGui::SameLine();
	//		ImGui::Checkbox("##visible", &lay->visible);

	//		ImGui::SameLine();
	//		ImGui::Image((void*)(intptr_t)icon.getTextureId(), ImVec2(selHei, selHei));

	//		ImGui::SameLine();
	//		auto posBef = ImGui::GetCursorPos();


	//		if (allowChange)
	//		{
	//			ImGui::BeginDisabled(fistLayer);
	//			if (ImGui::Button(ICON_FA_ANGLE_UP "", iconSize))
	//			{
	//				toMove[0] = prevId;
	//				toMove[1] = curID;
	//			}
	//			ImGui::EndDisabled();

	//			ImGui::SetCursorPos({ posBef.x, posBef.y + iconSize.y });
	//			ImGui::BeginDisabled(lastLayer);
	//			if (ImGui::Button(ICON_FA_ANGLE_DOWN "", iconSize))
	//			{
	//				toMove[0] = curID;
	//				catchNext = true;
	//			}
	//			ImGui::EndDisabled();

	//			ImGui::SetCursorPos({ posBef.x + iconSize.x, posBef.y });
	//			if (ImGui::Button(ICON_FA_TRASH "", ImVec2(selHei, selHei)))
	//			{
	//				delId = curID;
	//				lay->getCore()->release(proj->getMeta());
	//				if (curRadio)
	//				{
	//					if (lastLayer)
	//						displayRadioId = prevId;
	//					else
	//						incrementWorkingLayer = true;
	//				}

	//				if (iol.in == curID)
	//				{
	//					if (lastLayer)
	//						iol.in = prevId;
	//					else
	//						incrementCurrent = true;
	//				}
	//			}
	//			ImGui::SameLine();
	//		}

	//		bool seled = ImGui::Selectable(lay->getName(), curID == iol.in, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(winsize.x - 50, selHei));
	//		prevId = curID;

	//		if (seled)
	//		{
	//			iol.in = lay->getSysId();
	//			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
	//			{
	//				lay->lockAtThis(lastRealSize);
	//			}
	//		}
	//		if (updateWorkingImage)
	//		{
	//			if (j == 0)
	//				displayRadioId = -1;
	//			else
	//				displayRadioId = lay->getSysId();

	//			updateWorkingImage = false;
	//		}

	//		//if (j == iol.out)
	//		//{
	//		//	ImGui::PopFont();
	//		//}
	//		// ImVec4 color(1.0f, 0.0f, 0.0f, 1.0f); // RGBA color (red in this case)
	//		ImGui::PopID();
	//		++itemCounter;
	//	}

	//	//if (delId != -1)
	//	//{
	//	//	layers.remove(delId);
	//	//	proj->removeLayer(delId);
	//	//}

	//	//ImGui::EndGroup();

	///*	if (toMove[0] != -1 && toMove[1] != -1)
	//	{
	//		proj->layers.move(toMove[0], toMove[1]);
	//		layers.move(toMove[0], toMove[1]);
	//	}*/
	//}
	//

	void drawLayersWindow()
	{
		if (!ImGui::Begin("Слои"))
		{
			ImGui::End();
			return;
		}

		ImVec2 pos = ImGui::GetWindowPos();

		auto winsize = ImGui::GetWindowSize();
		winsize.y -= 50;

		bool updateWorkingImage = false;
		int& displayRadioId = iol.out;
		if (layers.size() > 0 && iol.out == -1)
			displayRadioId = (*layers.begin())->getSysId();

		float selHei = 40;
		ImVec2 iconSize(40, selHei / 2);

		bool catchNext = false, incrementCurrent = false, incrementWorkingLayer = false;
		int toMove[2]{ -1, -1 };
		int prevId = -1;
		int delId = -1;

		if (ImGui::BeginListBox("##LayersList", winsize))
		{
			if (displayRadioId == -1)
				displayRadioId = 0;

			buint j = 0;
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

				if (incrementCurrent)
				{
					incrementCurrent = false;
					iol.in = curID;
				}

				if (incrementWorkingLayer)
				{
					incrementWorkingLayer = false;
					displayRadioId = curID;
				}

				auto& icon = *lay->getIcon();

				bool curRadio = (displayRadioId == lay->getSysId());
				updateWorkingImage = ImGui::RadioButton("##Work", curRadio);//, curRadio);

				ImGui::SameLine();
				ImGui::Checkbox("##visible", &lay->getCore()->visible);

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
					if (curRadio)
					{
						if (lastLayer)
							displayRadioId = prevId;
						else
							incrementWorkingLayer = true;
					}

					if (iol.in == curID)
					{
						if (lastLayer)
							iol.in = prevId;
						else
							incrementCurrent = true;
					}
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
				if (updateWorkingImage)
				{
					if (j == 0)
						displayRadioId = -1;
					else
						displayRadioId  = lay->getSysId();

					updateWorkingImage = false;
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

		if (toMove[0] != -1 && toMove[1] != -1)
		{
			proj->layers.move(toMove[0], toMove[1]);
			layers.move(toMove[0], toMove[1]);
		}
		ImGui::End();
	}
};
