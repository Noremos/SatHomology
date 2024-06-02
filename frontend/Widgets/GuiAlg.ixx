module;
#include "../DrawCommon.h"

export module GuiAlgo;

//import BackBind;
import IAlgorithm;
import DynamicSettings;
import GuiFilter;
import IGuiLayer;

export class GuiAlg
{
	GuiFilter filtere;
	IAlg* alg;
public:
	GuiAlg()
	{

	}

	void set(IAlg* alg)
	{
		this->alg = alg;
	}

	void draw(ILayerWorker& context)
	{
		auto& factory = AlgFactory::getRasterFactory();

		static int selectedId = 0;
		if (selectedId > factory.functions.size())
		{
			selectedId = factory.functions.size() - 1;
		}


		static int selectedCatId = 0;
		static BackString selectedCatName;
		if (selectedCatName.length() == 0)
		{
			for (auto cat : factory.categories)
			{
				if (cat.second == selectedCatId)
				{
					selectedCatName = cat.first;
					break;
				}

			}
		}

		if (ImGui::BeginCombo("Category", selectedCatName.data()))
		{
			for (auto cat : factory.categories)
			{
				bool is_selected = (selectedCatId == cat.second);
				if (ImGui::Selectable(cat.first.data(), is_selected))
				{
					selectedCatName = cat.first;
					if (selectedCatId != cat.second)
					{
						selectedCatId = cat.second;
						for (size_t i = 0; i < factory.functions.size(); i++)
						{
							auto& p = factory.functions[i];
							if (p.categoryId != selectedCatId)
								continue;

							selectedId = i;
							break;
						}
					}
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		bool found = false;
		int foundId = -1;
		if (ImGui::BeginCombo("Algs", factory.functions[selectedId].name.data()))
		{
			for (size_t i = 0; i < factory.functions.size(); i++)
			{
				auto& p = factory.functions[i];
				if (p.categoryId != selectedCatId)
					continue;

				if (foundId == -1)
					foundId = i;

				bool is_selected = (selectedId == i); // You can store your selection however you want, outside or inside your objects
				if (ImGui::Selectable(factory.functions[i].name.data(), is_selected))
					selectedId = i;

				if (is_selected)
				{
					found = true;
					ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
				}
			}
			if (!found)
			{
				selectedId = foundId;
			}
			ImGui::EndCombo();
		}
		set(factory.get(selectedId));


		auto* settings = alg->getSettings();
		if (settings)
			drawDynamicSettings(*settings);

		if (alg->hasFilter())
		{
			if (ImGui::Button("Фильтр"))
			{
				ImGui::OpenPopup("FilterModal");
			}

			if (ImGui::BeginPopupModal("FilterModal", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				filtere.draw();
				if (ImGui::Button("Done"))
				{
					alg->setFilter(filtere.getFilter());
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		if (ImGui::Button("Run"))
		{
			auto rets = alg->execute(context.iol);
			context.setLayers(rets, "Разложить");
		}
	}
};
