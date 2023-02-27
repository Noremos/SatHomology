#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include <imgui_internal.h>
#include "GuiCommon.h"
#include <algorithm>

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
		values.push_back(value);
		++size;
	}

	void endAdding()
	{
		for (size_t i = 0; i < holder.size(); i++)
		{
			items.push_back(holder[i].c_str());
		}
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

	GuiImage& operator=(const GuiImage& other) noexcept = delete;
	GuiImage& operator=(GuiImage&& other) noexcept = delete;

	float getZoom()
	{
		return zoom.x;
	}
	WindowVec2 offset = ImVec2(0, 0);
	ItemVec2 clickedPos = ImVec2(0, 0);
	bool clicked = false;

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
		ImGuiWindowFlags window_flags;
		window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking;
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
		if (getTextureId() == 0)
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
			zoom.x = zoom.x > 0.1f ? zoom.x : 0.1f;
			zoom.y = zoom.y > 0.1f ? zoom.y : 0.1f;

			offset.x += abs(ads) * 20;
			offset.y += abs(ads) * 20;
		}
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
		ImGui::Image((void*)(intptr_t)getTextureId(), nsize);

		ImGui::SetScrollX(offset.x);
		ImGui::SetScrollY(offset.y);
	}

private:
	void drawTexture(ImGuiWindow* win, bool zoomable)
	{
		auto textId = getTextureId();
		if (textId == 0)
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
			ImGui::Image((void*)(intptr_t)textId, ImVec2(newWid, newHei));
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
	//bool dropPoints = false;
public:
	GuiDrawImage* par = nullptr;
	const bc::barvector* points = nullptr;

	//void setPoints(bc::barvector* _point, bool drop)
	//{
	//	if (dropPoints)
	//		delete points;

	//	points = _point;
	//	dropPoints = drop;
	//}

	//~GuiDrawCloudPointClick()
	//{
	//	setPoints(nullptr, false);
	//}


	void draw(const char* name, ImVec2 prevWin, ImVec2 scrollOff, ImVec2 winSize)
	{
		if (points == nullptr)
			return;

		//const ImVec2 p0 = ImGui::GetItemRectMin();
		//const ImVec2 p1 = ImGui::GetItemRectMax();
		//const char* text_str = "Line 1 hello\nLine 2 clip me!";
		//const ImVec2 text_pos = ImVec2(p0.x + offset.x, p0.y + offset.y);
		//ImDrawList* draw_list = ImGui::GetWindowDrawList();
		//switch (n)
		//{
		//case 0:
		//	ImGui::PushClipRect(p0, p1, true);
		//	draw_list->AddRectFilled(p0, p1, IM_COL32(90, 90, 120, 255));
		//	draw_list->AddText(text_pos, IM_COL32_WHITE, text_str);
		//	ImGui::PopClipRect();
		//	break;
		//case 1:
		//	draw_list->PushClipRect(p0, p1, true);
		//	draw_list->AddRectFilled(p0, p1, IM_COL32(90, 90, 120, 255));
		//	draw_list->AddText(text_pos, IM_COL32_WHITE, text_str);
		//	draw_list->PopClipRect();
		//	break;
		//case 2:
		//	ImVec4 clip_rect(p0.x, p0.y, p1.x, p1.y); // AddText() takes a ImVec4* here so let's convert.
		//	draw_list->AddRectFilled(p0, p1, IM_COL32(90, 90, 120, 255));
		//	draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize(), text_pos, IM_COL32_WHITE, text_str, NULL, 0.0f, &clip_rect);
		//	break;
		//}
		// ptoj->getVector();

		ImDrawList* list = ImGui::FindWindowByName(name)->DrawList;

		ImColor bigColor(128, 0, 255);
		ImColor midColor(220, 200, 0);

		float zoom = par->getZoom();
		ApplicationVec2 offset = prevWin + par->localDisplayPos;
		ApplicationVec2 csreenStar = prevWin + ImVec2(3 * zoom + 5, 3 * zoom + 5);
		ApplicationVec2 csreenEnd = prevWin + winSize - ImVec2(3 * zoom + 10, 3 * zoom + 10);
		const bc::barvector& pointsi = *points;

		float pixelSize = MAX(1,winSize.x / par->width);
		for (const auto& p : pointsi)
		{
			// TL is a Begin()
			ItemVec2 pi(par->toDisplayX(p.getX()), par->toDisplayY(p.getY()));
			pi -= scrollOff; // Admit the scrollers
			pi += offset; // TL coords from app

			if (pi.x < csreenStar.x || pi.y < csreenStar.y)
				continue;
			if (pi.x > csreenEnd.x || pi.y > csreenEnd.y)
				continue;

			// Center pixel for big images
			pi += ImVec2(pixelSize / 2, pixelSize / 2);
			list->AddCircleFilled(pi, 3 * pixelSize, bigColor);
			list->AddCircleFilled(pi, 2 * pixelSize, midColor);
		}
	}
};