module;
#include "../DrawCommon.h"

export module GuiAlgo;

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
		if (selectedId > factory.names.size())
		{
			selectedId = factory.names.size() - 1;
		}

		if (ImGui::BeginCombo("Algs", factory.names[selectedId].c_str()))
		{
			for (size_t i = 0; i < factory.names.size(); i++)
			{
				bool is_selected = (selectedId == i); // You can store your selection however you want, outside or inside your objects
				if (ImGui::Selectable(factory.names[i].c_str(), is_selected))
					selectedId = i;

				if (is_selected)
					ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
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
