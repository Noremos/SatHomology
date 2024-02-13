module;
#include <algorithm>
#include "DrawCommon.h"

export module GuiClassifierModule;

import BackTypes;
import BackBind;
import MatrModule;

import Platform;
import GuiWidgets;
import TrainIO;
import StatChart;
import DrawUtils;

import ClassifierCore;
import ProjectModule;


export struct GuiClassifer
{
	ClassifierBackend core;
	struct TrainPiecePreview
	{
		size_t dbLocalId;
		GuiImage img;
		TrainPiecePreview() : dbLocalId(-1)
		{ }

		TrainPiecePreview(const BackImage& inimg, size_t locId) : dbLocalId(locId)
		{
			img.setImage(inimg, false);
		}
	};

	struct GuiClass
	{
		int classId;
		std::vector<TrainPiecePreview> imgs;
		GuiClass(int id) : classId(id)
		{ }
	};

	SelectableKeyValues<GuiClass> classesLB;

	BarChart graph;
	BarChart graphBarlines;

	bool show = false;
	CachedObjectId selceted;
	int inLayer;

	ImVec4 curColor;
	bool changeColor = false;

	struct SelPair
	{
		size_t dbId = -1;
		int vecId = -1;
		bool hasData() const
		{
			return vecId != -1;
		}
	};

	SelPair selectedPrevied;
	char buffer[200];

	GuiClassifer()
	{
		memset(buffer, 0, 200);
		curColor = ImVec4(1.0, 1.f, 1.f, 1.f);
	}


	//int getClassImagesSize()
	//{
	//	return classImages.size();
	//}
	//ClassTextures& getClassImagesData()
	//{
	//	return classImages[classesLB.currentValue().vecId];
	//}

	ImVec2 getPngSize()
	{
		return ImVec2(32, 32);
	}

	void setCurrentClassName()
	{
		auto& t = classesLB.currentName();
		memcpy(buffer, t.c_str(), t.length());
	}

	void init()
	{
		classesLB.clear();

		ClassDataIO io;
		io.open(Project::proj->getMetaPath(core.classifier->name()));

		for (auto &&c : core.classCategs.categs)
		{
			classesLB.add(c.name, {c.id});
			loadClassImages(io, classesLB.back());
		}
		classesLB.endAdding();

		if (classesLB.getSize() > 0)
			setCurrentClassName();
	}

	void loadClassImages(ClassDataIO& io, GuiClass& classDisplayData)
	{
		ClassDataIO::TrainCallback cla = [&classDisplayData](int, vbuffer&, BackImage preview, size_t dbLocalId)
		{
			classDisplayData.imgs.push_back(TrainPiecePreview(preview, dbLocalId));
		};

		io.loadAll(cla, classDisplayData.classId, ClassDataIO::LF_ICON);
	}

	BackColor& getCurColor()
	{
		int claId = classesLB.currentValue().classId;
		return core.classCategs.get(claId)->color;
	}

	void drawClassifierWindow()
	{
		if (!show)
			return;

		graph.draw();
		graphBarlines.draw("Chart - barlines");

		if (!ImGui::Begin("Классификатор", &show))
		{
			ImGui::End();
			return;
		}
		ImGui::InputText("Имя", buffer, 200);

		const bool emptyName = strlen(buffer) == 0;
		ImGui::BeginDisabled(emptyName);
		if (ImGui::Button("Добавить"))
		{
			BackString st(buffer);
			int classId = core.addClassType(st);

			classesLB.add(st, {classId});
			classesLB.endAdding();

			if (classesLB.getSize() == 1)
			{
				setCurrentClassName();
			}
		}
		ImGui::EndDisabled();

		bool hasClasses = classesLB.getSize() != 0;
		ImGui::BeginDisabled(!hasClasses);

		ImGui::SameLine();

		ImGui::BeginDisabled(emptyName);
		if (ImGui::Button("Изменить"))
		{
			BackString st(buffer);
			int selectedClass = classesLB.currentValue().classId;
			classesLB.updateName(classesLB.currentIndex, st);
			core.changeClassName(selectedClass, st);
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Удалить"))
		{
			int selClassID = classesLB.currentValue().classId;
			core.removeClassType(selClassID);
			classesLB.remove(classesLB.currentIndex);
		}
		ImGui::EndDisabled();

		hasClasses = classesLB.getSize() != 0; // Check it again after the dropping

		classesLB.drawListBox("Категории");
		if (classesLB.hasChanged())
		{
			setCurrentClassName();
			const auto& col = getCurColor();
			curColor.x = static_cast<float>(col.r) / 255.f;
			curColor.y = static_cast<float>(col.g) / 255.f;
			curColor.z = static_cast<float>(col.r) / 255.f;
		}

		//if (ImGui::Button("Load from image"))
		//{
		//	ImGui::OpenPopup("LoadImg");
		//	//backend.undoAddClass();
		//}

		if (ImGui::BeginPopupModal("LoadImg", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (ImGui::Button("Load iamge"))
			{
			}
			ImGui::SameLine();

			//ImGui::SetNextWindowPos(pos);
			//ImGui::SetNextWindowSize(size);
			//prview.drawImage("Processed");
			//ImGui::Separator();

			ImGui::Separator();
			//auto id = ImGui::FindWindowByName("ProcSetts")->ID;
			//tbVals.tilePrview.draw(id, tbVals.getTileSize(), tbVals.getOffsetSize(), tbVals.getImageSize());
			//if (ImGui::IsItemHovered())
			//	ImGui::SetTooltip("I am a tooltip over a popup");

			//static int unused_i = 0;
			//ImGui::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

			ImGui::Separator();
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}

		if (hasClasses)
		{
			ImGui::Checkbox("Изменить цвет", &changeColor);
			if (changeColor)
			{
				ImGui::SameLine();
				if (ImGui::Button("Применить"))
				{
					auto& col = getCurColor();
					col.r = std::min(static_cast<int>(curColor.x * 255), 255);
					col.g = std::min(static_cast<int>(curColor.y * 255), 255);
					col.b = std::min(static_cast<int>(curColor.z * 255), 255);
				}

				ImGui::ColorPicker4("Цвет", (float*)&curColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs);
			}

			//if (ImGui::Button("Show graph"))
			//{
			//	int classId = classesLB.currentValue().classId;
			//	graph.init(core.classifier.dbPath, classId);
			//	graphBarlines.initBarlines(core.classifier.dbPath, classId);
			//}

			ImGui::BeginDisabled(!selceted.hasData());
			ImGui::SameLine();
			if (ImGui::Button("Добавить выбранный"))
			{
				BackImage icon;
				auto& selectedClass = classesLB.currentValue();
				assert(core.classCategs.size() !=0);
				size_t dbLocalId = core.addTrainData(inLayer, selectedClass.classId, selceted, &icon);
				selectedClass.imgs.push_back(TrainPiecePreview(icon, dbLocalId));
			}
			ImGui::EndDisabled();

			ImGui::BeginDisabled(!selectedPrevied.hasData());
			ImGui::SameLine();
			if (ImGui::Button("Удалить данные"))
			{
				auto& selectedClass = classesLB.currentValue();
				core.removeTrainData(selectedClass.classId, selectedPrevied.dbId);
				auto& b = selectedClass.imgs;
				b.erase(b.begin() + selectedPrevied.vecId);
				selectedPrevied.vecId = -1;
			}
			ImGui::EndDisabled();


			ImGui::BeginGroup();

			// int selHei = getPngSize().y;
			auto selSize = getPngSize() * 1.3;
			float off = (selSize.x - getPngSize().x) / 2;
			//auto& pngs = getClassImagesData().imgs;

			float winWid = ImGui::GetCurrentWindow()->Size.x;
			if (ImGui::BeginListBox("##ClassDataList", ImVec2(winWid, 0)))
			{
				ImVec2 posAfter;

				int j = 0;
				auto& prevImgs = classesLB.currentValue().imgs;

				float stX = 0;
				for (auto& icon : prevImgs)
				{
					ImGui::PushID(j);

					ImVec2 posBef;
					if (j != 0)
					{
						ImGui::SetCursorPos(posAfter);
						posBef = posAfter;
					}
					else
					{
						posBef = ImGui::GetCursorPos();
						stX = posBef.x;
					}

					bool seled = ImGui::Selectable("", icon.dbLocalId == selectedPrevied.dbId, 0, selSize);
					if (seled)
						selectedPrevied = {icon.dbLocalId, j};

					ImGui::SetCursorPos(posBef + ImVec2(off, off));
					ImGui::Image(icon.img.getTexturePtr(), getPngSize());

					float newXoff = posBef.x + selSize.x + 3;
					if (newXoff >= winWid)
					{
						posAfter = { stX, posAfter.y + selSize.y };
					}
					else
						posAfter = { newXoff, posBef.y };

					//ImGui::SetCursorPos(posAfter);

					//ImGui::SameLine();

					ImGui::PopID();
					++j;
				}
				ImGui::EndListBox();
			}
			ImGui::EndGroup();
		}
		ImGui::End();
	}
};
