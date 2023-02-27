module;
#include "GuiWidgets.h"


export module LayersGui;

import LayersCore;
//import ForntnedModule;
import BarcodeModule;


class LayersVals;

export class GuiLayer
{
public:
	//std::unique_ptr<ILayer> coreData;
	int id;
	GuiImage icon;
	BackString name;
	ILayer layer;

	virtual void draw(LayersVals& context) = 0;

private:
	virtual void toGuiData() = 0;
};


export class RasterGuiLayer : public GuiLayer
{
public:
	RasterLayer data;
	//bool showHighmap = false;
	GuiDrawImage main;

	virtual void draw(LayersVals&)
	{
		main.drawImage(name.c_str());
	}

private:
	virtual void toGuiData()
	{
		main.setImage(data.mat, false);
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


	template<typename T>
	T* addMainLayer()
	{
		curLayerId = 0;
		temporalyId = -1;
		layers.clear();
		return layers.add<T>();
	}

	template<typename T>
	T* addLayer()
	{
		auto t = layers.add<T>();
		curLayerId = temporalyId;
		temporalyId = layers.size();
		return t;
	}

	void draw(const ImVec2 pos, const ImVec2 size)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		//size.x -= leftProps;
		//size.x /= 2;


		uint i = 0;
		for (auto& lay : layers)
		{
			ImGui::SetNextWindowPos(pos);
			ImGui::SetNextWindowSize(size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushID(i);
			lay->draw(*this);
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

		if (ImGui::BeginListBox("LayersList"))
		{
			uint j = 0;
			for (auto& lay : layers)
			{
				ImGui::PushID(j);

				if (j == temporalyId)
				{
					ImFont* italicFont = ImGui::GetIO().Fonts->Fonts[1];  // Assuming the second font in the ImFontAtlas is italic
					ImGui::PushFont(italicFont);
				}
				
				ImGui::Selectable(lay->name.c_str(), false, 0, ImVec2(wid, 30));

				if (j == temporalyId)
				{
					ImGui::PopFont();
				}

				auto& icon = lay->icon;
				ImGui::Image((void*)(intptr_t)icon.getTextureId(), ImVec2(icon.width, icon.height));
				ImGui::SameLine();
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



export class RasterLineGuiLayer : public GuiLayer
{
public:
	RasterLineLayer data;

	GuiDrawImage main;
	BackString debug;
	SimpleLine* selecedLine = nullptr;
	GuiDrawCloudPointClick clickHandler;

	RasterLineGuiLayer()
	{
		clickHandler.par = &main;
	}

	virtual void draw(LayersVals& context)
	{
		ImVec2 prevOff = main.offset;
		ImVec2 prevwWin = main.winPos;
		main.drawImage(name.c_str(), true);
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
		if (data.clickResponser.size() == 0)
			return nullptr;

		x = main.getRealX(x);
		y = main.getRealY(y);


		if (x < 0 || x >= main.width)
			return NULL;

		if (y < 0 || y >= main.height)
			return NULL;

		//std::cout << x << " : " << y << std::endl;

		SimpleLine* line = data.clickResponser[y * main.width + x].get();
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

		if (!ImGui::Begin("Свойства"))
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
			ImGui::Text("Достугнут конец");
		}

		ImGui::End();
	}

private:
	virtual void toGuiData()
	{
		main.setImage(data.mat, false);
	}
};
