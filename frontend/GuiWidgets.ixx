module;

#include "GuiCommon.h"

#include "imgui.h"
#include <imgui_internal.h>

#include <algorithm>

export module GuiWidgets;

export using ApplicationVec2 = ImVec2;
export using WindowVec2 = ImVec2; // The top-left is a begin window
export using ItemVec2 = ImVec2; // The top-left is a Item (image) coords

export template<class T>
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

	void remove(int id)
	{
		holder.erase(holder.begin() + id);
		values.erase(values.begin() + id);
		--size;
		endAdding();

		if (id <= currentIndex)
		{
			--currentIndex;
		}
	}

	void updateName(int index, const BackString& name)
	{
		holder[index] = name;
		items[index] = holder[index].c_str();
	}

	void endAdding()
	{
		items.clear();
		for (size_t i = 0; i < holder.size(); i++)
		{
			items.push_back(holder[i].c_str());
		}
	}

	void clear()
	{
		size = 0;
		holder.clear();
		values.clear();
	}

	const char** getItems()
	{
		return items.data();
	}

	T currentValue()
	{
		return values[currentIndex];
	}

	const BackString& currentName()
	{
		return holder[currentIndex];
	}

	void drawCombobox(const char* name, int width = 200)
	{
		int oldId = currentIndex;
		const char** items = getItems();
		ImGui::SetNextItemWidth(width);
		ImGui::Combo(name, &currentIndex, items, size); //IM_ARRAYSIZE(items)

		hasChange = oldId != currentIndex;
	}

	void drawListBox(const char* name, int width = 200)
	{
		int oldId = currentIndex;
		const char** items = getItems();
		ImGui::SetNextItemWidth(width);
		ImGui::ListBox(name, &currentIndex, items, size); //IM_ARRAYSIZE(items)

		hasChange = oldId != currentIndex;
	}

	T* getValuesIterator()
	{
		return values.data();
	}

	int getSize()
	{
		return size;
	}

	bool hasChanged()
	{
		return hasChange;
	}

private:
	std::vector<const char*> items;
	std::vector<T> values;
	std::vector<BackString> holder;
	int size;
	bool hasChange;
};


export class GuiDrawInfo
{
	ImVec2 pos;
	ImVec2 size;
};

export class GuiResizableContainer
{
	ImVec2 zoom = ImVec2(1, 1);
public:
	ImVec2 displaySize;
	WindowVec2 offset = ImVec2(0, 0);
	ItemVec2 clickedPos = ImVec2(0, 0);

	GuiImage& operator=(const GuiImage& other) noexcept = delete;
	GuiImage& operator=(GuiImage&& other) noexcept = delete;

	float getZoom()
	{
		return zoom.x;
	}

	bool clicked = false;

	bool good;
	bool Begin(const char* name)
	{
		auto window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking;
		window_flags |= ImGuiWindowFlags_HorizontalScrollbar;
		if (!ImGui::Begin(name, NULL, window_flags))
		{
			good = false;
		}
		else
			good = true;

		return good;
	}

	void end(ImVec2, ImVec2 realSize)
	{
		if (!good)
			return;

		ImVec2 wpos = ImGui::GetCurrentWindow()->Pos;
		const bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

		clicked = false;
		bool drgged = false;
		bool wheeled = false;
		if (hovered)
		{
			drgged = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
			clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Right);
			wheeled = ImGui::GetIO().MouseWheel != 0;
		}
		if (clicked)
		{
			clickedPos = ImGui::GetIO().MousePos - wpos + offset;
		}

		offset.x = ImGui::GetScrollX();
		offset.y = ImGui::GetScrollY();

		if (drgged)
		{
			offset.x -= ImGui::GetIO().MouseDelta.x;
			offset.y -= ImGui::GetIO().MouseDelta.y;
		}
		if (wheeled)
		{
			float ads = ImGui::GetIO().MouseWheel * 0.1f;
			zoom.x += ads;
			zoom.y += ads;
			zoom.x = zoom.x > 0.1f ? zoom.x : 0.1f;
			zoom.y = zoom.y > 0.1f ? zoom.y : 0.1f;

			offset.x += abs(ads) * 20;
			offset.y += abs(ads) * 20;
		}


		ImVec2 nsize = ImVec2((float)realSize.x * zoom.x, (float)realSize.y * zoom.y);
		displaySize = nsize;

		ImGui::SetScrollX(offset.x);
		ImGui::SetScrollY(offset.y);

		ImGui::End();
	}
};


export class GuiDrawImage : public GuiImage
{
public:
	WindowVec2 localDisplayPos;
	ImVec2 displaySize;
	virtual ~GuiDrawImage()
	{ }

	GuiImage& operator=(const GuiImage& other) noexcept = delete;
	GuiImage& operator=(GuiImage&& other) noexcept = delete;


	int getRealX(int x)
	{
		return static_cast<float>(x - localDisplayPos.x) * (width / displaySize.x);
	}
	int getRealY(int y)
	{
		return static_cast<float>(y - localDisplayPos.y) * (height / displaySize.y);
	}

	int toDisplayX(int x)
	{
		return static_cast<float>(x) * (displaySize.x / width) + localDisplayPos.x;
	}
	int toDisplayY(int y)
	{
		return static_cast<float>(y) * (displaySize.y / height) + localDisplayPos.y;
	}

	void drawImage(const char* name, ImVec2 pos, ImVec2 size)
	{
		auto window_flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove;

		ImGui::SetCursorPos(pos);
		if (ImGui::BeginChild(name, size, false, window_flags))
		{
			drawTexture(pos, size);
		}
		ImGui::EndChild();
	}

private:
	void drawTexture(ImVec2 pos, ImVec2 size)
	{
		auto textId = getTextureId();
		if (textId == 0)
		{
			ImDrawList* list = ImGui::GetWindowDrawList();
			localDisplayPos.x = 5;
			localDisplayPos.y = 5;
			ImVec2 maxPos = pos;
			maxPos.x += size.x - 10;
			maxPos.y += size.y - 30;

			ImVec2 minPos = pos + localDisplayPos;
			list->AddRectFilled(minPos, maxPos, ImColor(115, 140, 153));
			//ImGui::Image((void*)0, size);
			return;
		}

		int newWid = width;
		int newHei = height;

		ResizeImage(newWid, newHei, size.x, size.y);
		this->scaleFactor = width / static_cast<float>(newWid);
		displaySize.x = newWid;
		displaySize.y = newHei;

		localDisplayPos.x = (size.x - newWid) / 2;
		localDisplayPos.y = (size.y - newHei) / 2;
		ImGui::SetCursorPos(localDisplayPos);
		ImGui::Image((void*)(intptr_t)textId, ImVec2(newWid, newHei));
	}
};


export class GuiTilePreview
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
		// ImGui::ShowMetricsWindow();

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
		window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

		ImGuiWindow* win = ImGui::FindWindowByID(parentId);
		//if (ImGui::BeginChild(parentId))
		{
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


export class GuiDrawCloudPointClick : public GuiImage
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


	void draw(ImVec2 pos, ImVec2 winSize)
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

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
		window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

		ImGui::SetCursorPos(pos);
		if (!ImGui::BeginChild("DrawClick", winSize, false, window_flags))
		{
			ImGui::EndChild();
			return;
		}

		auto* win = ImGui::GetCurrentWindow();
		ImDrawList* list = win->DrawList;

		ImColor bigColor(128, 0, 255);
		ImColor midColor(220, 200, 0);

		ApplicationVec2 offset = win->Pos + pos;
		ApplicationVec2 csreenStar = offset + par->localDisplayPos;
		ApplicationVec2 csreenEnd = offset + par->localDisplayPos + par->displaySize;
		float markerSize = 5;//MAX(1, par->displaySize.x / par->width);
		float pixelSize = par->displaySize.x / par->width;

		const bc::barvector& pointsi = *points;
		for (const auto& p : pointsi)
		{
			// TL is a Begin()
			ItemVec2 pi(par->toDisplayX(p.getX()), par->toDisplayY(p.getY()));
			pi += offset; // TL coords from app

			if (pi.x < csreenStar.x || pi.y < csreenStar.y)
				continue;
			if (pi.x > csreenEnd.x || pi.y > csreenEnd.y)
				continue;

			// Center pixel for big images
			pi += ImVec2(pixelSize / 2, pixelSize / 2);
			list->AddCircleFilled(pi, 1.5 * markerSize, bigColor);
			list->AddCircleFilled(pi, 0.8 * markerSize, midColor);
		}

		ImGui::EndChild();
	}
};
