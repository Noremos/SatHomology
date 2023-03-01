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
public:
	virtual void draw(LayersVals& context, ImVec2 pos, ImVec2 size) = 0;
	virtual const char* getName() = 0;
	virtual GuiImage* getIcon() = 0;

	virtual void toGuiData() = 0;

	virtual int getSysId() = 0;
	virtual const char* getStrId()
	{
		return strId.c_str();
	}
};


export template<class T>
class GuiLayerData : public GuiLayer
{
public:
	T* data;
	GuiLayerData()
	{
		data = proj->addLayerData<T>();
	}

	GuiLayerData(T* fromCore)
	{
		data = fromCore;
	}

	virtual int getSysId()
	{
		return data->id;
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
		val->setName("Main");
		val->toGuiData();

		curLayerId = val->data->id;
		temporalyId = -1;
		return val;
	}

	template<typename T>
	T* addLayer(const BackString& name)
	{
		auto t = layers.add<T>();
		curLayerId = temporalyId = t->data->id;
		t->setName(name);
		return t;
	}

	template<typename TGui, typename TData>
	TGui* addLayer(const BackString& name, TData* core)
	{
		if (core == nullptr)
			return nullptr;

		auto t = layers.add<TGui>(core);
		curLayerId = temporalyId = t->data->id;
		t->setName(name);
		t->toGuiData();
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
			ptr->setName(name);
			return nullptr;
		}
		delete ptr;

		return addLayer<TGui, TData>(name, core);
	}

	void draw(const ImVec2 pos, const ImVec2 size)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		//size.x -= leftProps;
		//size.x /= 2;


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


	void drawLayersWindow()
	{
		if (!ImGui::Begin("Layers"))
		{
			ImGui::End();
			return;
		}


		ImDrawList* drawList = ImGui::GetWindowDrawList();
		float wid = ImGui::GetWindowWidth();
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

		if (ImGui::BeginListBox("##LayersList"))
		{
			uint j = 0;
			for (auto& lay : layers)
			{
				ImGui::PushID(j);

				//if (lay->getSysId()s == temporalyId)
				//{
				//	ImFont* italicFont = ImGui::GetIO().Fonts->Fonts[1];  // Assuming the second font in the ImFontAtlas is italic
				//	ImGui::PushFont(italicFont);
				//}

				auto& icon = *lay->getIcon();
				auto pos = ImGui::GetCursorPos();
				ImGui::Image((void*)(intptr_t)icon.getTextureId(), ImVec2(icon.width, icon.height));
				ImGui::SameLine();
				ImGui::Selectable(lay->getName(), false, 0, ImVec2(wid, 30));

				//if (j == temporalyId)
				//{
				//	ImGui::PopFont();
				//}

				//ImGui::Text(lay.name.c_str());
				//ImVec2 pos(50, 50);
				float size = 100.0f;
				ImVec4 color(1.0f, 0.0f, 0.0f, 1.0f); // RGBA color (red in this case)

				// Draw a filled rectangle with the specified color
				//drawList->AddRectFilled(pos, ImVec2(pos.x + wid, pos.y + lay.height), ImGui::GetColorU32(color));
				ImGui::PopID();
				++j;
			}
			ImGui::EndListBox();
		}

		//ImGui::EndGroup();

		ImGui::End();
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

	virtual void draw(LayersVals& context, ImVec2 pos, ImVec2 size)
	{
		ImVec2 prevOff = main.offset;
		ImVec2 prevwWin = main.winPos;
		main.drawImage(data->name.c_str(), pos, size);
		if (main.clicked)
		{
			auto p = main.clickedPos;

			auto points = click(p.x, p.y);
			setPoints(points);
		}
		debug = intToStr(ImGui::GetIO().MousePos.x) + ":" + intToStr(ImGui::GetIO().MousePos.y);
		clickHandler.draw("Processed", prevwWin, prevOff, context.drawSize);
		drawPropertisWindow();
	}


	void setPoints(const bc::barvector* points)
	{

		if (points == nullptr)
		{
			clickHandler.points = points;
			return;
		}

		clickHandler.points = points;

		//if (backend.getAlg() == 0)
		//{
		//	getCountourSimple(*points, reservPoints);
		//	clickHandler.points = &reservPoints;
		//}
		//else
		//	clickHandler.points = points;
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
			/*std::cout << line->matr.size() * 100.f / mainMat.length() << std::endl;
			MatrImg temp;
			temp.assignCopyOf(resultMart);
			auto &fullmatr = line->matr;

			for (int i = 0, total = fullmatr.size(); i < total; ++i)
			{
				auto p = bc::barvalue::getStatPoint(fullmatr[i].index);
				temp.set(p.x, p.y, Barscalar(255, 191, 0));
			}

			processedImage->setImage(temp);*/
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

		if (!ImGui::Begin("��������"))
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
			ImGui::Text("��������� �����");
		}

		ImGui::End();
	}

private:
};
