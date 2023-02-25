module;

#include "GuiCommon.h"
#include "imgui.h"
#include <imgui_internal.h>

export module LayersGui;


export struct Layer
{
	GuiImage icon;
	GuiImage realImage;
	BackString name;
};

export struct LayersVals
{
	int curLayer = 0;
	std::vector<Layer> layers;
	int selecnedLayer = 0;

	Layer& getCurrentLayer()
	{
		return layers[curLayer];
	}

	void drawLayer()
	{
		//if (!ImGui::Ite("Layers"))
		//{
		//	ImGui::End();
		//	return;
		//}
	}

	void drawLayers()
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

		if (ImGui::BeginListBox("ImagePreview"))
		{
			for (size_t j = 0; j < pngs.size(); j++)
			{
				ImVec2 pos = ImGui::GetWindowPos();

				ImGui::Selectable(pngs[j].name.c_str());
				auto& icon = pngs[j].icon;
				ImGui::PushID(j);
				ImGui::Image((void*)(intptr_t)icon.getTextureId(), ImVec2(icon.width, icon.height));
				ImGui::SameLine();
				//ImGui::Text(pngs[j].name.c_str());
				//ImVec2 pos(50, 50);
				float size = 100.0f;
				ImVec4 color(1.0f, 0.0f, 0.0f, 1.0f); // RGBA color (red in this case)

				// Draw a filled rectangle with the specified color
				//drawList->AddRectFilled(pos, ImVec2(pos.x + wid, pos.y + pngs[j].height), ImGui::GetColorU32(color));
				ImGui::PopID();
			}
			ImGui::EndListBox();
		}

		//ImGui::EndGroup();

		ImGui::End();
	}
};
