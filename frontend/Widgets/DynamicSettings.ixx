module;
#include "../GuiCommon.h"

export module DynamicSettings;

import IOCore;
import MLSettings;


static int MyResizeCallback(ImGuiInputTextCallbackData* data)
{
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		BackString* my_str = (BackString*)data->UserData;
		IM_ASSERT(my_str->c_str() == data->Buf);
		my_str->resize(data->BufSize); // NB: On resizing calls, generally data->BufSize == data->BufTextLen + 1
		data->Buf = (char*)my_str->c_str();
	}
	return 0;
}

// Note: Because ImGui:: is a namespace you would typically add your own function into the namespace.
// For example, you code may declare a function 'ImGui::InputText(const char* label, MyString* my_str)'
static bool MyInputText(const char* label, BackString* my_str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0)
{
	IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
	return ImGui::InputText(label, (char*)my_str->c_str(), my_str->length(), flags | ImGuiInputTextFlags_CallbackResize, MyResizeCallback, (void*)my_str);
}

export void drawDynamicSettings(MLSettings& settings)
{
	for (auto& set : settings.values)
	{
		const char* label = set.name.c_str();
		switch (set.type)
		{
		case OptionValue::sv_bool:
			ImGui::Checkbox(label, &set.data.b);
			break;
		case OptionValue::sv_int:
			ImGui::InputInt(label, &set.data.i);
			break;
		case OptionValue::sv_double:
			ImGui::InputDouble(label, &set.data.d);
			break;
		case OptionValue::sv_str:
			MyInputText(label, set.data.s);
			break;
		case OptionValue::sv_enum:
			if (ImGui::BeginCombo(label, set.data.e->getSelected().data()))
			{
				for (size_t i = 0; i < set.data.e->values.size(); i++)
				{
					bool is_selected = (set.data.e->selected == i); // You can store your selection however you want, outside or inside your objects
					if (ImGui::Selectable(set.data.e->values[i].c_str(), is_selected))
						set.data.e->selected = i;

					if (is_selected)
						ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
				}
				ImGui::EndCombo();
			}
			break;
		case OptionValue::sv_path:
			break;
		}
	}
	ImGui::Separator();
}