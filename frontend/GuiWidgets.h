#pragma once

#include "imgui.h"
#include <imgui_internal.h>
#include "GuiCommon.h"

template<class T>
struct ComboKeyValues
{
	int currentIndex = 0;

	ComboKeyValues(std::initializer_list<std::pair<T, const char*>> l)
	{
		size = l.size();
		items.reset(new const char* [size]);
		values.reset(new T[size]);
		int counter = 0;
		for (const auto& kv : l)
		{
			values[counter] = kv.first;
			items[counter] = kv.second;
			++counter;
		}
	}

	const char** getItems()
	{
		return items.get();
	}

	T currentValue()
	{
		return values[currentIndex];
	}

	void drawCombobox(const char* name, int width = 200)
	{
		const char** items = getItems();
		ImGui::SetNextItemWidth(width);
		ImGui::Combo(name, &currentIndex, items, size); //IM_ARRAYSIZE(items)
	}

private:
	std::unique_ptr<const char* []> items;
	std::unique_ptr<T[]> values;
	int size;
};


class GuiDrawImage : public GuiImage
{
public:
	virtual ~GuiDrawImage()
	{ }

	int tileSize = 0;

	void drawImage(const char* name)
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
		window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

		if (ImGui::Begin(name, nullptr, window_flags))
		{
			ImGuiWindow* win = ImGui::FindWindowByName(name);
			drawTexture(win);
			ImGui::End();
		}
	}

private:
	void drawTexture(ImGuiWindow* win)
	{
		if (textureId == 0)
		{
			ImDrawList* list = ImGui::GetWindowDrawList();
			ImVec2 startPos = win->Pos;
			startPos.x += 5;
			startPos.y += 5;
			ImVec2 maxPos = win->Pos;
			maxPos.x += win->Size.x - 10;
			maxPos.y += win->Size.y - 30;
			list->AddRectFilled(startPos, maxPos, ImColor(115, 140, 153));
			//ImGui::Image((void*)0, win->Size);
			return;
		}

		int newWid = width;
		int newHei = height;

		ResizeImage(newWid, newHei, win->Size.x, win->Size.y - 20);
		this->scaleFactor = width / static_cast<float>(newWid);

		ImVec2 localSet((win->Size.x - newWid) / 2, (win->Size.y - newHei) / 2);
		ImGui::SetCursorPosX(localSet.x);
		ImGui::SetCursorPosY(localSet.y);
		ImGui::Image((void*)(intptr_t)textureId, ImVec2(newWid, newHei));

		ImVec2 cont =  ImGui::GetCursorPos();
		if (tileSize)
		{
			ImDrawList* list = ImGui::GetWindowDrawList();

			ImVec2 drawTileSize(tileSize / scaleFactor, tileSize / scaleFactor);
			ImVec2 maxPos = win->Pos;
			maxPos.x += localSet.x;
			maxPos.y += localSet.y;

			ImColor lineColor(115, 115, 115);

			for (float w = 0; w < newWid; w += drawTileSize.x)
			{
				ImVec2 ts(maxPos.x + w, maxPos.y);
				ImVec2 te(maxPos.x + w, maxPos.y + newHei);

				list->AddLine(ts, te, lineColor, 3);
			}
			ImVec2 ts(maxPos.x + newWid, maxPos.y);
			ImVec2 te(maxPos.x + newWid, maxPos.y + newHei);
			list->AddLine(ts, te, lineColor, 3);

			for (float h = 0; h < newHei; h += drawTileSize.y)
			{
				ImVec2 ts(maxPos.x, maxPos.y + h);
				ImVec2 te(maxPos.x + newWid, maxPos.y + h);
				list->AddLine(ts, te, lineColor, 3);
			}
			ImVec2 tse(maxPos.x, maxPos.y + newHei);
			ImVec2 tee(maxPos.x + newWid, maxPos.y + newHei);
			list->AddLine(tse, tee, lineColor, 3);
		}

		ImGui::SetCursorPos(cont);
	}

};


class GuiTilePreview
{
	int getAddnl(int main, int minor)
	{
		//  (ImageWidth + TileWidth - 1) / TileWidth
		//	return (main / minor) + (main % minor == 0 ? 0 : 1);
		return (main + minor - 1) / minor;
	}
public:
	void draw(ImGuiID parentId, int tileSize, int offset, ImVec2 imgSize)
	{
		//ImGui::ShowDemoWindow();
		ImGui::ShowMetricsWindow();

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
		window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

		ImGuiWindow* win = ImGui::FindWindowByID(parentId);
		//if (ImGui::BeginChild(parentId))
		//if (ImGui::Begin("MyDraw",NULL, window_flags))
		{
			//win = ImGui::FindWindowByName("MyDraw");

			//ImDrawList* list = win->DrawList;// ImGui::GetWindowDrawList();

			int y = getAddnl(imgSize.x, tileSize) * getAddnl(imgSize.y, tileSize);
			ImGui::Text("The image will be splitted into %d tiles", y);

			ImDrawList* list = ImGui::GetWindowDrawList();

			ImVec2 localStartPos = ImGui::GetCursorPos();
			//startPos.x += 5;
			//startPos.y += 5;
			ImVec2 maxPos;
			maxPos.x = win->Size.x;
			maxPos.y = 200;
			int drawWid = imgSize.x, drawHei = imgSize.y;
			ResizeImage(drawWid, drawHei, maxPos.x, maxPos.y);
			float scale = imgSize.x / drawWid;

			// Draw List use the coores from top beootm corner of the BASE window. So add the modal wins coords
			ImVec2 absolutePos(win->Pos.x + localStartPos.x, win->Pos.y + localStartPos.y);
			list->AddRectFilled(absolutePos, ImVec2(absolutePos.x + drawWid, absolutePos.y + drawHei), ImColor(150, 150, 150));

			ImVec2 tileWithOffSize((tileSize + offset) / scale, (tileSize + offset) / scale);
			list->AddRectFilled(absolutePos, ImVec2(absolutePos.x + tileWithOffSize.x, absolutePos.y + tileWithOffSize.y), ImColor(80, 80, 80));


			ImVec2 drawTileSize(tileSize / scale, tileSize / scale);
			list->AddRectFilled(absolutePos, ImVec2(absolutePos.x + drawTileSize.x, absolutePos.y + drawTileSize.y), ImColor(50, 50, 50));
			if (drawTileSize.x >= 1)
			{
				for (float w = 0; w < drawWid; w += drawTileSize.x)
				{
					ImVec2 ts(absolutePos.x + w, absolutePos.y);
					ImVec2 te(absolutePos.x + w, absolutePos.y + drawHei);

					list->AddLine(ts, te, ImColor(15, 15, 15));
				}
			}

			if (drawTileSize.y >= 1)
			{
				for (float h = 0; h < drawHei; h += drawTileSize.y)
				{
					ImVec2 ts(absolutePos.x, absolutePos.y + h);
					ImVec2 te(absolutePos.x + drawWid, absolutePos.y + h);

					list->AddLine(ts, te, ImColor(15, 15, 15));
				}
			}

			localStartPos.y += drawHei + 5;
			ImGui::SetCursorPos(localStartPos);

			//ImGui::End();
			//ImGui::EndChild();
		}
	}
};