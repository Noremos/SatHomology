module;
#include "GuiWidgets.h"


export module LayersGui;

import LayersCore;
import ProjectModule;
import BarcodeModule;


class LayersVals;

Project* proj = Project::getProject();

export class GuiLayer
{
protected:
	BackString strId;
	int copiedId = -1; // We need to cpy Id to avoid cases when core was destroid
public:
	bool visible = true;

	virtual void draw(LayersVals& context, ImVec2 pos, ImVec2 size) = 0;
	virtual const char* getName() = 0;
	virtual GuiImage* getIcon() = 0;

	virtual void toGuiData() = 0;

	virtual int getSysId()
	{
		return copiedId;
	}
	virtual const char* getStrId()
	{
		return strId.c_str();
	}

	virtual void onClick(ImVec2 pos)
	{

	}
};


export template<class T>
class GuiLayerData : public GuiLayer
{
protected:
	T* data;
public:
	GuiLayerData()
	{
		data = proj->addLayerData<T>();
		copiedId = data->id;
	}

	GuiLayerData(T* fromCore)
	{
		data = fromCore;
		copiedId = data->id;
	}

	T* getData()
	{
		return data;
	}

	virtual GuiImage* getIcon()
	{
		return &icon;
	}

	virtual const char* getName()
	{
		return data->name.c_str();
	}


	void setName(const BackString& name)
	{
		data->name = name;
		strId = name + intToStr(data->id);
	}

	GuiImage icon;
};

export class RasterGuiLayer : public GuiLayerData<RasterLayer>
{
public:
	//bool showHighmap = false;
	GuiDrawImage main;

	RasterGuiLayer() : GuiLayerData()
	{ }

	RasterGuiLayer(RasterLineLayer* fromCore) : GuiLayerData(fromCore)
	{ }

	virtual void draw(LayersVals&, ImVec2 pos, ImVec2 size)
	{
		if (!visible)
			return;

		main.drawImage(data->name.c_str(), pos, size);
	}

	virtual void toGuiData()
	{
		main.setImage(data->mat, false);
		icon.setImage(data->mat, 32, 32, true);
	}
};

export class LayersVals
{
public:
	int temporalyId = -1;
	int curLayerId = -1;

	//GuiBackend& backend;
	LayersList<GuiLayer> layers;

	ImVec2 drawSize;


	//LayersVals(GuiBackend& back) : backend(back)
	//{ }

	RasterGuiLayer* getMainImg()
	{
		return static_cast<RasterGuiLayer*>(layers.at(0));
	}

	GuiLayer* getCurrentLayer()
	{
		return curLayerId < 0 ? nullptr : layers.at(curLayerId);
	}

	GuiLayer* getTempLayer()
	{
		return temporalyId < 0 ? nullptr : layers.at(temporalyId);
	}

	void update()
	{
		if (temporalyId == -1)
			return;
	}

	template<typename T>
	T* addMainLayer()
	{
		layers.clear();
		proj->layers.clear();
		T* val = layers.add<T>(&proj->main);
		settup(val, "Main");
		temporalyId = -1;
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
		curLayerId = temporalyId = layer->getSysId();
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
			lay->draw(*this, pos, size);
			ImGui::PopID();
			++i;
		}

		//pos.x += drawSize.x;
		//ImGui::SetNextWindowPos(pos);
		//ImGui::SetNextWindowSize(nextSize);
		//ImGui::SetNextWindowViewport(viewport->ID);
	}

	void onClick(ImVec2 click)
	{
		if (curLayerId == -1)
			return;
		auto layer = layers.at(curLayerId);
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


		ImDrawList* drawList = ImGui::GetWindowDrawList();
		auto winsize = ImGui::GetWindowSize();
		ImVec2 pos = ImGui::GetWindowPos();

		auto& pngs = layers;

		//int curItem = 0;
		//auto imgLoader = [](void* data, int idx, const char** out_text)
		//{
		//	auto& pngs = *static_cast<std::vector<Layer>*>(data);

		//	auto& icon = pngs[idx].icon;

		//	*out_text = pngs[idx].name.c_str();
		//	ImGui::Image(icon.getTexturePtr(), ImVec2(icon.width, icon.height));
		//	ImGui::SameLine();
		//	return true;
		//};

		//ImGui::ListBox("Images", &selecnedLayer, imgkLoader, &pngs, pngs.size());


		//{
		//	if (curItem >= 0 && selectedImage < pngs.size())
		//	{
		//		ImVec2 siz(pngs[curItem].width, pngs[curItem].height)
		//		ImGui::Image(pngs[curItem].getTexturePtr(), siz);
		//	}
		//	ImGui::EndChild();
		//}
		winsize.y -= 50;

		bool temporaly = false;
		int displayRadioId = temporalyId;
		if (layers.size() > 0 && temporalyId == -1)
			displayRadioId = (*layers.begin())->getSysId();

		int selHei = 40;
		ImVec2 iconSize(40, selHei / 2);


		bool catchNext = false;
		int toMove[2]{ -1, -1 };
		int prevId = -1;
		if (ImGui::BeginListBox("##LayersList", winsize))
		{
			uint j = 0;
			for (auto& lay : layers)
			{

				//if (lay->getSysId()s == temporalyId)
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
				temporaly = ImGui::RadioButton("Work", curRadio);//, curRadio);

				ImGui::SameLine();
				ImGui::Checkbox("##visible", &lay->visible);

				ImGui::SameLine();
				ImGui::Image((void*)(intptr_t)icon.getTextureId(), ImVec2(selHei, selHei));
				ImGui::SameLine();

				ImGui::BeginDisabled(j == 0 || j == 1);
				auto posBef = ImGui::GetCursorPos();
				if (ImGui::Button(ICON_FA_ANGLE_UP "", iconSize))
				{
					toMove[0] = prevId;
					toMove[1] = curID;
				}

				if (j==1)
					ImGui::EndDisabled();

				ImGui::SetCursorPos({posBef.x, posBef.y + iconSize.y});
				if (ImGui::Button(ICON_FA_ANGLE_DOWN "", iconSize))
				{
					toMove[0] = curID;
					catchNext = true;
				}
				if (j != 1)
					ImGui::EndDisabled();

				ImGui::SetCursorPos({posBef.x + iconSize.x, posBef.y});


				bool seled = ImGui::Selectable(lay->getName(), curID == curLayerId, 0, ImVec2(winsize.x - 50, selHei));
				prevId = curID;

				if (seled)
				{
					curLayerId = lay->getSysId();
				}
				if (temporaly)
				{
					if (j == 0)
						temporalyId = -1;
					else
						temporalyId = lay->getSysId();

					displayRadioId = lay->getSysId();
					temporaly = false;
				}

				//if (j == temporalyId)
				//{
				//	ImGui::PopFont();
				//}
				ImVec4 color(1.0f, 0.0f, 0.0f, 1.0f); // RGBA color (red in this case)
				ImGui::PopID();
				++j;
			}
			ImGui::EndListBox();
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



export class RasterLineGuiLayer : public GuiLayerData<RasterLineLayer>
{
public:
	GuiDrawImage main;
	BackString debug;
	SimpleLine* selecedLine = nullptr;
	GuiDrawCloudPointClick clickHandler;

	RasterLineGuiLayer() : GuiLayerData()
	{
		clickHandler.par = &main;
	}

	RasterLineGuiLayer(RasterLineLayer* fromCore) : GuiLayerData(fromCore)
	{
		clickHandler.par = &main;
	}

	virtual void toGuiData()
	{
		main.setImage(data->mat, false);
		icon.setImage(data->mat, 32, 32, true);
	}

	ImVec2 drawSize;
	virtual void draw(LayersVals& context, ImVec2 pos, ImVec2 size)
	{
		if (!visible)
			return;

		drawSize = context.drawSize;
		main.drawImage(data->name.c_str(), pos, size);

		ImVec2 prevOff = main.offset;
		ImVec2 prevwWin = main.winPos;
		clickHandler.draw(pos, size);
		drawPropertisWindow();
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
			if (selecedLine == line && line->parent)
				line = line->parent.get();

			selecedLine = line;
			return &selecedLine->matr;
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
		return selecedLine = selecedLine->parent.get();
	}

	void drawPropertisWindow()
	{
		SimpleLine* line = selecedLine;
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
