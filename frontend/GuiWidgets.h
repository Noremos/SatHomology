#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include <imgui_internal.h>
#include "GuiCommon.h"

using ApplicationVec2 = ImVec2;
using WindowVec2 = ImVec2; // The top-left is a begin window
using ItemVec2 = ImVec2; // The top-left is a Item (image) coords

template<class T>
struct SelectableKeyValues
{
	int currentIndex = 0;

	SelectableKeyValues() : size(0)
	{ }

	SelectableKeyValues(std::initializer_list<std::pair<T, const char*>> l)
	{
		size = l.size();
		items.resize(size);
		values.resize(size);
		int counter = 0;
		for (const auto& kv : l)
		{
			values[counter] = kv.first;
			items[counter] = kv.second;
			++counter;
		}
	}

	void add(const BackString& name, T value)
	{
		holder.push_back(name);
		items.push_back(holder.back().c_str());
		values.push_back(value);
	}

	const char** getItems()
	{
		return items.data();
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

	void drawListBox(const char* name, int width = 200)
	{
		const char** items = getItems();
		ImGui::SetNextItemWidth(width);
		ImGui::ListBox(name, &currentIndex, items, size); //IM_ARRAYSIZE(items)
	}

	T* getValuesIterator()
	{
		return values.data();
	}

	int getSize()
	{
		return size;
	}

private:
	std::vector<const char*> items;
	std::vector<T> values;
	std::vector<BackString> holder;
	int size;
};


class GuiDrawInfo
{
	ImVec2 pos;
	ImVec2 size;
};

class GuiDrawImage : public GuiImage
{
	ImVec2 zoom = ImVec2(1, 1);
public:
	WindowVec2 localDisplayPos;
	ApplicationVec2 winPos;
	ImVec2 displaySize;
	virtual ~GuiDrawImage()
	{ }

	float getZoom()
	{
		return zoom.x;
	}
	WindowVec2 offset = ImVec2(0, 0);
	ItemVec2 clickedPos = ImVec2(0, 0);
	bool clicked = false;
	int tileSize = 0;


	int getRealX(int x)
	{
		return static_cast<float>(x) * (width / displaySize.x);
	}
	int getRealY(int y)
	{
		return static_cast<float>(y) * (height / displaySize.y);
	}

	int toDisplayX(int x)
	{
		return static_cast<float>(x) / (width / displaySize.x);
	}
	int toDisplayY(int y)
	{
		return static_cast<float>(y) / (height / displaySize.y);
	}

	void drawImage(const char* name, bool zoomable = false)
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
		window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
		if (zoomable)
			window_flags |= ImGuiWindowFlags_HorizontalScrollbar;
		if (ImGui::Begin(name, nullptr, window_flags))
		{
			ImGuiWindow* win = ImGui::FindWindowByName(name);
			winPos = win->Pos;
			drawTexture(win, zoomable);
		}
		ImGui::End();
	}

	void checkZoom(ImVec2 realSize)
	{
		if (textureId == 0)
			return;
		//void show_zoomable_image(unsigned char* image_data, int width, int height, int channels) {

		//ImGui::Image(image_data, ImVec2((float)width, (float)height), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		// Handle zoom events
		offset.x = ImGui::GetScrollX();
		offset.y = ImGui::GetScrollY();
		if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			offset.x -= ImGui::GetIO().MouseDelta.x;
			offset.y -= ImGui::GetIO().MouseDelta.y;
		}
		if (ImGui::IsWindowHovered() && ImGui::GetIO().MouseWheel != 0)
		{
			float ads = ImGui::GetIO().MouseWheel * 0.1f;
			zoom.x += ads;
			zoom.y += ads;
			zoom.x = std::max(zoom.x, 0.1f);
			zoom.y = std::max(zoom.y, 0.1f);

			offset.x += abs(ads) * 20;
			offset.y += abs(ads) * 20;
		}
		ImGui::SetScrollX(offset.x);
		ImGui::SetScrollY(offset.y);

		//ImVec2 pmin = ImGui::GetCursorScreenPos();
		//ImVec2 pmax(ImGui::GetCursorScreenPos().x + width * zoom.x, ImGui::GetCursorScreenPos().y + height * zoom.y);
		//ImVec2 uvMin(offset.x / width, offset.y / height);
		//ImVec2 uvMax((offset.x + width * zoom.x) / width, (offset.y + height * zoom.y) / height);
		//ImGui::GetWindowDrawList()->AddImage((void*)(intptr_t)textureId, pmin, pmax, uvMin, uvMax);

		//ImVec2 size = ImVec2((float)width * zoom.x, (float)height * zoom.y);
		//ImVec2 uv0 = ImVec2((float)offset.x / width, (float)offset.y / height);
		//ImVec2 uv1 = ImVec2((float)(offset.x + size.x) / width, (float)(offset.y + size.y) / height);
		//ImGui::Image((void*)(intptr_t)textureId, size, uv0, uv1);

		ImVec2 nsize = ImVec2((float)realSize.x * zoom.x, (float)realSize.y * zoom.y);
		displaySize = nsize;
		ImGui::Image((void*)(intptr_t)textureId, nsize);
	}

private:
	void drawTexture(ImGuiWindow* win, bool zoomable)
	{
		if (textureId == 0)
		{
			ImDrawList* list = ImGui::GetWindowDrawList();
			localDisplayPos.x = 5;
			localDisplayPos.y = 5;
			ImVec2 maxPos = win->Pos;
			maxPos.x += win->Size.x - 10;
			maxPos.y += win->Size.y - 30;

			ImVec2 minPos = win->Pos + localDisplayPos;
			list->AddRectFilled(minPos, maxPos, ImColor(115, 140, 153));
			//ImGui::Image((void*)0, win->Size);
			return;
		}

		int newWid = width;
		int newHei = height;

		ResizeImage(newWid, newHei, win->Size.x, win->Size.y - 30);
		this->scaleFactor = width / static_cast<float>(newWid);
		displaySize.x = newWid;
		displaySize.y = newHei;

		localDisplayPos.x = (win->Size.x - newWid) / 2;
		localDisplayPos.y = (win->Size.y - newHei) / 2;
		ImGui::SetCursorPosX(localDisplayPos.x);
		ImGui::SetCursorPosY(localDisplayPos.y);

		if (zoomable)
		{
			clicked = false;
			if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				clickedPos = ImGui::GetIO().MousePos - win->Pos - localDisplayPos + offset;
				clicked = true;
			}

			checkZoom(ImVec2(newWid, newHei));
		}
		else
			ImGui::Image((void*)(intptr_t)textureId, ImVec2(newWid, newHei));

		ImVec2 cont =  ImGui::GetCursorPos();
		if (tileSize)
		{
			ImDrawList* list = ImGui::GetWindowDrawList();

			ImVec2 drawTileSize(tileSize / scaleFactor, tileSize / scaleFactor);
			ImVec2 maxPos = win->Pos + localDisplayPos;

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


class GuiDrawCloudPointClick : public GuiImage
{
public:
	bc::barvector* points = nullptr;
	GuiDrawImage* par = nullptr;
	void draw()
	{
		if (points == nullptr)
			return;
		// ptoj->getVector();

		ImDrawList* list = ImGui::GetWindowDrawList();

		ImColor bigColor(255, 0, 0);
		ImColor midColor(220, 200, 0);

		float zoom = par->getZoom();
		ApplicationVec2 offset = par->winPos + par->localDisplayPos;
		bc::barvector& pointsi = *points;
		for (const auto& p : pointsi)
		{
			// TL is a Begin()
			WindowVec2 pi = par->localDisplayPos + ItemVec2(par->toDisplayX(p.getX()), par->toDisplayY(p.getY()));
			pi -= par->offset; // Admit the scrollers

			//pi -= par->offset;
			list->AddCircleFilled(offset + pi, 3 * zoom, bigColor);
			list->AddCircleFilled(offset + pi, 2 * zoom, midColor);
		}
	}
};