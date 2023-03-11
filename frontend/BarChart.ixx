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
	static const int len = 300;
	struct DrawChartline
	{
		DrawChartline(const char* name, int totalSize, int max) :
			name(name), op(1.0f / totalSize), proc(static_cast<float>(max) / len)
		{
			std::fill(oppas, oppas + len, 0);
		}

		//int drawDid;
		//int metrictStart;
		//int metrictEnd;
		BackString name;
		float op;
		float proc;
		bool empty = true;
		float oppas[len + 1];

		void set(int metrStart, int metrLen)
		{
			empty = false;

			//const float proc = 300.f / (metrStart + metrLen);
			int st = metrStart / proc;// 300.f;
			int ed = (metrStart + metrLen) / proc;// 300.f;
			ed = MIN(ed, len);

			for (int i = st; i <= ed; i++)
			{
				oppas[i] += op;
			}
		}

		void draw(ImDrawList* list)
		{
			ImVec2 pos = ImGui::GetCursorScreenPos();
			ImGui::Text(name.c_str());
			ImVec2 posAfter = ImGui::GetCursorScreenPos();
			float hei = posAfter.y - pos.y;
			float sad = 3;
			hei = hei - sad * 2;

			// ImGui::SameLine();
			//ImVec2 pos = ImGui::GetCursorScreenPos();
			pos.x += 70;
			pos.y += sad;

			//ImVec2 size(len, 20);
			//ImVec2 end = pos + size;
			int stSegment = 0;
			float stColr = oppas[0];
			for (int i = 1; i < len; i++)
			{
				if (stColr != oppas[i])
				{
					ImColor col(1.0f, 1.0f, 1.0f, stColr);

					ImVec2 spos = pos;
					spos.x += stSegment;

					ImVec2 epos = pos;
					epos.x += i;
					epos.y += hei;
					list->AddRectFilled(spos, epos, col);
					stSegment = i;
					stColr = oppas[i];
				}
			}
		}
	};

public:
	std::unique_ptr<ImDrawListSharedData> dummy_data;
	std::unique_ptr<ImDrawList> cached_draw_list;

	std::vector<DrawChartline> draws;

	int clId = -1;
	void init(const BackPathStr& dbPath, int classId)
	{
		clId = classId;
		//dummy_data.reset(new ImDrawListSharedData());
		//cached_draw_list.reset(new ImDrawList(dummy_data.get()));
		draws.clear();

		ClassDataIO io;
		io.open(dbPath);
		int count = io.getClassCount(classId);

		int counter = 0;
		const int depthC = counter++;
		draws.push_back({ "Depth", count, 10 });
		const int startC = counter++;
		draws.push_back({ "start", count, 256 });
		const int startrC = counter++;
		draws.push_back({ "start r", count, 256 });
		const int startgC = counter++;
		draws.push_back({ "start g", count, 256 });
		const int startbC = counter++;
		draws.push_back({ "start b", count, 256 });
		const int endC = counter++;
		draws.push_back({ "end", count, 256 });
		const int endrC = counter++;
		draws.push_back({ "end r", count, 256 });
		const int endgC = counter++;
		draws.push_back({ "end g", count, 256 });
		const int endbC = counter++;
		draws.push_back({ "end b", count, 256 });
		const int lenC = counter++;
		draws.push_back({ "len", count, 256 });
		const int matrsizeC = counter++;
		draws.push_back({ "mart size", count, 1000});


		ClassDataIO::TrainCallback cla = [this, endC, matrsizeC, lenC](int, vbuffer& buf, BackImage, size_t dbLocalId)
		{
			std::ostringstream oss;
			oss.write(reinterpret_cast<const char*>(buf.data()), buf.size());

			std::istringstream iss(oss.str());

			int counter = 0;

			BarlineClass raw;
			raw.read(iss);

			draws[counter++].set(0, raw.getDeath());
			auto rstart = raw.start();
			if (rstart.type == BarType::BYTE8_1)
			{
				draws[counter++].set(0, rstart.data.b1);
			}
			else
			{
				counter++;
				draws[counter++].set(0, rstart.data.b3[0]);
				draws[counter++].set(0, rstart.data.b3[1]);
				draws[counter++].set(0, rstart.data.b3[2]);
			}

			counter = endC;

			auto rend = raw.end();
			if (rend.type == BarType::BYTE8_1)
			{
				draws[counter++].set(0, rend.data.b1);
			}
			else
			{
				counter++;

				draws[counter++].set(0, rend.data.b3[0]);
				draws[counter++].set(0, rend.data.b3[1]);
				draws[counter++].set(0, rend.data.b3[2]);
			}

			draws[lenC].set(0, (rend - rstart).getAvgFloat());
			draws[matrsizeC].set(0, raw.getMatrixSize());
		};


		ImDrawListSplitter splitter;
		//splitter.Split(cached_draw_list.get(), 0); // Start new layer
		//for (auto&& c : proj->classCategs.categs)
		{
			io.loadAll(cla, classId, ClassDataIO::LF_BINFILE);
		}
		//splitter.Merge(cached_draw_list.get()); // End layer
	}


	void drawPlei()
	{
		ImDrawList* list = ImGui::GetWindowDrawList();

		ImVec2 rf(15, 15);
		ImVec2 pos = ImGui::GetCursorPos();

		for (size_t i = 0; i < draws.size(); i++)
		{
			auto d = draws[i];
			if (!d.empty)
				continue;

			ImGui::SetCursorPos(pos);
			d.draw(list);

			pos.y += 30;
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
		ImDrawList* list = window->DrawList;
		ImVec2 offset = window->Pos;

		ImVec2 rf(15, 15);
		ImVec2 pos = ImGui::GetCursorPos();
		pos += rf;

		for (size_t i = 0; i < draws.size(); i++)
		{
			auto d = draws[i];
			if (d.empty)
				continue;

			// ImGui::SetCursorPos(pos);
			d.draw(list);

			pos.y += 30;
		}

		ImGui::End();
	}
};
