module;
#include "GuiCommon.h"

export module LayersGui;

import RasterLayers;
import ProjectModule;
import BarcodeModule;
import GuiWidgets;
import GuiOverlap;

class LayersVals;

Project* proj = Project::getProject();

export class GuiLayer
{
protected:
	BackString strId;
	int copiedId = -1; // We need to cpy Id to avoid cases when core was destroid
public:
	bool visible = true;

	virtual void draw(ImVec2 pos, ImVec2 size) = 0;
	virtual void drawOverlap(ImVec2 pos, ImVec2 size) = 0;
	virtual const char* getName() const = 0;
	virtual GuiImage* getIcon() = 0;

	virtual void toGuiData() = 0;

	virtual ILayer* getCore() = 0;
	virtual void drawProperty() = 0;
	virtual void applyPropertyChanges() = 0;

	virtual void onClick(ImVec2)
	{ }

	virtual int getSysId()
	{
		return copiedId;
	}
	virtual const char* getStrId()
	{
		return strId.c_str();
	}

	virtual ~GuiLayer()
	{ }
};


export template<class T>
class GuiLayerData : public GuiLayer
{
protected:
	T* data;

public:
	GuiLayerData(T* fromCore = nullptr)
	{
		if (fromCore == nullptr)
			data = proj->addLayerData<T>();
		else
			data = fromCore;

		copiedId = data->id;
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

	void setName(const BackString& name)
	{
		data->name = name;
		strId = name + intToStr(data->id);
	}

	GuiImage icon;

	virtual ~GuiLayerData()
	{ }
};


export template<class T>
class TiledRasterGuiLayer : public GuiLayerData<T>
{
public:
	GuiDrawImage main;

	TiledRasterGuiLayer(T* fromCore) : GuiLayerData<T>(fromCore)
	{ }

	virtual ~TiledRasterGuiLayer()
	{ }

	virtual void toGuiData()
	{
		newTileSize = getTileSize() / 10;
		newOffsetSize = getOffsetSize() / 10;
	}

	virtual void draw(ImVec2 pos, ImVec2 size)
	{
		if (!GuiLayer::visible)
			return;

		main.drawImage(GuiLayerData<T>::getName(), pos, size);
	}

	virtual void drawOverlap(ImVec2, ImVec2)
	{ }

	int newTileSize;
	int newOffsetSize;
	GuiTilePreview tilePrview;

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
		tilePrview.draw(id, getTileSize(), getOffsetSize(), getImageSize());
	}

	void applyPropertyChanges()
	{
		GuiLayerData<T>::data->prov.tileSize = newTileSize * 10;
		GuiLayerData<T>::data->tileOffset = newOffsetSize * 10;
	}
};


export class RasterGuiLayer : public TiledRasterGuiLayer<RasterLayer>
{
public:
	RasterGuiLayer(RasterLayer* fromCore = nullptr) : TiledRasterGuiLayer(fromCore)
	{ }

	virtual void toGuiData()
	{
		TiledRasterGuiLayer::toGuiData();

		main.setImage(data->mat, false);
		icon.setImage(data->mat, 32, 32, true);
	}
};

export class RasterLineGuiLayer : public TiledRasterGuiLayer<RasterLineLayer>
{
public:
	BackString debug;
	SimpleLine* selectedLine = nullptr;
	GuiDrawCloudPointClick clickHandler;

	RasterLineGuiLayer(RasterLineLayer* fromCore = nullptr) : TiledRasterGuiLayer(fromCore)
	{
		clickHandler.par = &main;
	}

	virtual void toGuiData()
	{
		TiledRasterGuiLayer::toGuiData();
		main.setImage(data->mat, false);
		icon.setImage(data->mat, 32, 32, true);
	}

	virtual void draw(ImVec2 pos, ImVec2 size)
	{
		TiledRasterGuiLayer::draw(pos, size);
		if (!visible)
			return;

		clickHandler.draw(pos, size);
		drawLineInfoWin();
	}

	virtual void onClick(ImVec2 pos)
	{
		// if (clicked)
		{
			auto points = click(pos.x, pos.y);
			setPoints(points);
		}
	}


	void setPoints(const bc::barvector* points)
	{
		if (points == nullptr)
		{
			clickHandler.points = points;
			return;
		}

		clickHandler.points = points;
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

	void drawLineInfoWin()
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
				setPoints(&line->matr);
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
	RasterFromDiskGuiLayer(RasterFromDiskLayer* fromCore = nullptr) : TiledRasterGuiLayer(fromCore)
	{ }

	virtual void toGuiData()
	{
		TiledRasterGuiLayer::toGuiData();

		auto i = *data->getCachedImage();
		main.setImage(i, false);
		icon.setImage(i, 32, 32, true);
	}

	std::vector<SubImgInf> getSubImageInfos()
	{
		return data->getSubImageInfos();
	}
};


export class LayersVals
{
public:
	InOutLayer iol;

	//GuiBackend& backend;
	LayersList<GuiLayer> layers;

	ImVec2 drawSize;


	//LayersVals(GuiBackend& back) : backend(back)
	//{ }

	InOutLayer* getIoLayer()
	{
		return &iol;
	}

	IRasterLayer* getCurrentRasterCore()
	{
		return iol.in < 0 ? nullptr : dynamic_cast<IRasterLayer*>(layers.at(iol.in)->getCore());
	}

	template<typename T>
	T* getCastCurrentLayer()
	{
		GuiLayer* l = getCurrentLayer();
		if (l)
			return dynamic_cast<T*>(l);
		else
			return nullptr;
	}

	GuiLayer* getCurrentLayer()
	{
		return iol.in < 0 ? nullptr : layers.at(iol.in);
	}

	GuiLayer* getTempLayer()
	{
		return iol.out < 0 ? nullptr : layers.at(iol.out);
	}

	void update()
	{
		if (iol.out == -1)
			return;
	}

	RasterFromDiskGuiLayer* addImageFromDiskLayer()
	{
		layers.clear();
		proj->layers.clear();
		RasterFromDiskGuiLayer* val = layers.add<RasterFromDiskGuiLayer>();
		settup(val, "From disk");
		iol.out = -1;
		return val;
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

	template<typename TGui>
	void settup(TGui* layer, const BackString& name)
	{
		iol.in = iol.out = layer->getSysId();
		layer->setName(name);
		layer->toGuiData();
	}


	void loadLayers()
	{
		// for(auto coreLay : proj->layers)
		// {
		// 	addLayer<
		// }
	}

	void draw(const ImVec2 pos, const ImVec2 size)
	{
		uint i = 0;
		for (auto& lay : layers)
		{
			// ImGui::SetNextWindowPos(pos);
			// ImGui::SetNextWindowSize(size);
			// ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushID(i);
			lay->draw(pos, size);
			ImGui::PopID();
			++i;
		}

		//pos.x += drawSize.x;
		//ImGui::SetNextWindowPos(pos);
		//ImGui::SetNextWindowSize(nextSize);
		//ImGui::SetNextWindowViewport(viewport->ID);
	}

	void drawOverlap(const ImVec2 pos, const ImVec2 size)
	{
		auto t = getCurrentLayer();
		if (t)
			t->drawOverlap(pos, size);
	}

	void onClick(ImVec2 click)
	{
		if (iol.in == -1)
			return;
		auto layer = layers.at(iol.in);
		if (layer)
			layer->onClick(click);
	}

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
				}

				ImGui::SameLine();
				bool seled = ImGui::Selectable(lay->getName(), curID == iol.in, 0, ImVec2(winsize.x - 50, selHei));
				prevId = curID;

				if (seled)
				{
					iol.in = lay->getSysId();
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
			proj->layers.remove(delId);
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
