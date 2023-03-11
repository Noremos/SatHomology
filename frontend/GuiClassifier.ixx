module;
#include "GuiCommon.h"

export module GuiClassifierModule;

import ProjectModule;
import IOCore;
import Platform;
import GuiWidgets;
import TrainIO;
import StatChart;

Project* proj = Project::getProject();


export struct GuiClassifer
{
	struct GuiClass
	{
		int classId;
		int vecId;
	};

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
	struct ClassPreview
	{
		std::vector<TrainPiecePreview> imgs;
	};

	SelectableKeyValues<GuiClass> classesLB;
	std::vector<ClassPreview> classImages;

	BarChart graph;

	bool show = false;
	CachedObjectId selceted;
	InOutLayer* ioLayer;

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
		io.open(proj->getMetaPath(proj->classifier.name()));

		for (auto &&c : proj->classCategs.categs)
		{
			classesLB.add(c.name, {c.id, (int)classImages.size()});
			loadClassImages(io, c.id);
		}
		classesLB.endAdding();

		if (classImages.size() > 0)
			setCurrentClassName();
	}

	void loadClassImages(ClassDataIO& io, int classId)
	{
		ClassPreview prev;
		ClassDataIO::TrainCallback cla = [&prev](int, vbuffer&, BackImage preview, size_t dbLocalId)
		{
			prev.imgs.push_back(TrainPiecePreview(preview, dbLocalId));
		};

		io.loadAll(cla, classId, ClassDataIO::LF_ICON);
		classImages.push_back(prev);
	}

	void drawClassifierWindow()
	{
		graph.draw();

		if (!ImGui::Begin("Classifier", &show))
		{
			ImGui::End();
			return;
		}
		ImGui::InputText("Name", buffer, 200);

		const bool emptyName = strlen(buffer) == 0;
		ImGui::BeginDisabled(emptyName);
		if (ImGui::Button("Add"))
		{
			BackString st(buffer);
			int classId = proj->addClassType(st);
			classesLB.add(st, {classId, (int)classImages.size()});
			classesLB.endAdding();

			classImages.push_back(ClassPreview());
			if (classImages.size() == 1)
			{
				setCurrentClassName();
			}
		}
		ImGui::EndDisabled();

		const bool hasClasses = classesLB.getSize() != 0;
		ImGui::BeginDisabled(!hasClasses);

		ImGui::SameLine();

		ImGui::BeginDisabled(emptyName);
		if (ImGui::Button("Edit"))
		{
			BackString st(buffer);
			int selectedClass = classesLB.currentValue().classId;
			classesLB.updateName(classesLB.currentIndex, st);
			proj->changeClassName(selectedClass, st);
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Drop"))
		{
			auto selectedClass = classesLB.currentValue();
			proj->removeClassType(selectedClass.classId);
			classImages.erase(classImages.begin() + selectedClass.vecId);
			classesLB.remove(classesLB.currentIndex);
		}
		ImGui::EndDisabled();

		classesLB.drawListBox("Категории");
		if (classesLB.hasChanged())
		{
			setCurrentClassName();
		}

		if (ImGui::Button("Load from image"))
		{
			ImGui::OpenPopup("LoadImg");
			//backend.undoAddClass();
		}

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
			if (ImGui::Button("Show graph"))
			{
				graph.init(proj->getMetaPath(proj->classifier.name()), classesLB.currentValue().classId);
			}

			ImGui::BeginDisabled(!selceted.hasData());
			ImGui::SameLine();
			if (ImGui::Button("Add selected"))
			{
				BackImage icon;
				auto selectedClass = classesLB.currentValue();
				assert(proj->classCategs.size() !=0);
				size_t dbLocalId = proj->addTrainData(ioLayer->in, selectedClass.classId, selceted, &icon);
				classImages[selectedClass.vecId].imgs.push_back(TrainPiecePreview(icon, dbLocalId));
			}
			ImGui::EndDisabled();

			ImGui::BeginDisabled(!selectedPrevied.hasData());
			ImGui::SameLine();
			if (ImGui::Button("Drop"))
			{
				auto selectedClass = classesLB.currentValue();
				proj->removeTrainData(selectedClass.classId, selectedPrevied.dbId);
				auto& b = classImages[selectedClass.vecId].imgs;
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
			if (ImGui::BeginListBox("##LayersList", ImVec2(winWid, 0)))
			{
				ImVec2 posAfter;

				int j = 0;
				int selectedClass = classesLB.currentValue().vecId;
				ClassPreview& prev = classImages[selectedClass];


				float stX = 0;
				for (auto& icon : prev.imgs)
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
