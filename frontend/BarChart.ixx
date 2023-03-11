module;
#include "GuiCommon.h"

export module StatChart;

import IOCore;
import LayersCore;
import GuiWidgets;

import RasterLayers;
import ClassifierInterface;
import TrainIO;
import Classifiers;
import ProjectModule;


Project* proj = Project::getProject();


export class BarChart
{
public:
	std::unique_ptr<ImDrawListSharedData> dummy_data;
	std::unique_ptr<ImDrawList> cached_draw_list;

	int clId = -1;
	void init(const BackPathStr& dbPath, int classId)
	{
		clId = classId;
		dummy_data.reset(new ImDrawListSharedData());
		cached_draw_list.reset(new ImDrawList(dummy_data.get()));

		ClassDataIO io;
		io.open(dbPath);
		//, io.getClassCount();

		ClassDataIO::TrainCallback cla = [this](int, vbuffer& buf, BackImage, size_t dbLocalId)
		{
			std::ostringstream oss;
			oss.write(reinterpret_cast<const char*>(buf.data()), sizeof(buf.size()));

			std::istringstream iss(oss.str());

			BarlineClass raw;
			raw.read(iss);
			ImVec2 rf(15, 15);
			drawPlei(dbLocalId, rf, 0, raw.getDeath(), "Depth");
			rf.y += 30;

			auto rstart = raw.start();
			if (rstart.type == BarType::BYTE8_1)
			{
				drawPlei(dbLocalId, rf, 0, raw.getDeath(), "start");
				rf.y += 30;
			}
			else
			{
				drawPlei(dbLocalId, rf, 0, rstart.data.b3[0], "start r");
				rf.y += 30;
				drawPlei(dbLocalId, rf, 0, rstart.data.b3[1], "start g");
				rf.y += 30;
				drawPlei(dbLocalId, rf, 0, rstart.data.b3[2], "start b");
				rf.y += 30;
			}

			auto rend = raw.end();
			if (rend.type == BarType::BYTE8_1)
			{
				drawPlei(dbLocalId, rf, 0, raw.getDeath(), "end");
				rf.y += 30;
			}
			else
			{
				drawPlei(dbLocalId, rf, 0, rend.data.b3[0], "end r");
				rf.y += 30;
				drawPlei(dbLocalId, rf, 0, rend.data.b3[1], "end g");
				rf.y += 30;
				drawPlei(dbLocalId, rf, 0, rend.data.b3[2], "end b");
				rf.y += 30;
			}

			drawPlei(dbLocalId, rf, 0, raw.getMatrixSize(), "matr size");
			rf.y += 30;
		};


		ImDrawListSplitter splitter;
		//splitter.Split(cached_draw_list.get(), 0); // Start new layer
		//for (auto&& c : proj->classCategs.categs)
		{
			io.loadAll(cla, classId, ClassDataIO::LF_BINFILE);
		}
		//splitter.Merge(cached_draw_list.get()); // End layer
	}


	void drawPlei(size_t totalSize, ImVec2 of, int start, int len, const char* name)
	{
		ImDrawList* list = ImGui::GetWindowDrawList();
		ImVec2 offset = ImGui::GetCurrentWindow()->Pos;

		ImVec2 size(300 / len, 20);
		ImColor col(0.0f, 1.0f, 1.0f, 1.0f / totalSize);

		// Render the window contents to the cached draw list
		{
			ImGui::SetCursorPos(offset + of);

			ImGui::Text(name);
			ImGui::SameLine();
			ImVec2 pos = ImGui::GetCursorPos();
			pos.x += 30;

			ImVec2 end = pos + size;

			cached_draw_list->AddRectFilled(pos, end, col);
		}
	}

	virtual void draw()
	{
		if (clId == -1)
			return;

		//auto window_flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollbar;
		if (!ImGui::Begin("BarChart"))//, size, false, window_flags))
		{
			ImGui::End();
			return;
		}

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImDrawList* draw_list = window->DrawList;


		// Render the cached draw list instead of the original draw list
		draw_list->CmdBuffer.swap(cached_draw_list->CmdBuffer);
		draw_list->IdxBuffer.swap(cached_draw_list->IdxBuffer);
		draw_list->VtxBuffer.swap(cached_draw_list->VtxBuffer);

		ImGui::End();
	}
};
