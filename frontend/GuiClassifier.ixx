module;
#include "GuiCommon.h"

export module GuiClassifierModule;

import ProjectModule;
import IOCore;
import Platform;
import GuiWidgets;
import TrainIO;

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
			img.setImage(inimg, true);
		}
	};
	struct ClassPreview
	{
		std::vector<TrainPiecePreview> imgs;
	};

	SelectableKeyValues<GuiClass> classesLB;
	std::vector<ClassPreview> classImages;
	bool show = false;
	CachedObjectId selceted;

	struct SelPair
	{
		size_t dbId = -1;
		int vecId = -1;
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

	void init()
	{
		classesLB.clear();

		ClassDataIO io;
		io.open(proj->getMetaPath(barclassificator::className()));

		for (auto &&c : proj->classCategs.categs)
		{
			classesLB.add(c.name, {c.id, (int)classImages.size()});
			loadClassImages(io, c.id);
		}

		classesLB.endAdding();
	}

	void loadClassImages(ClassDataIO& io, int classId)
	{
		ClassPreview prev;
		ClassDataIO::TrainCallback cla = [&prev](int, vbuffer, BackImage preview, size_t dbLocalId)
		{
			prev.imgs.push_back(TrainPiecePreview(preview, dbLocalId));
		};

		io.loadAll(cla, classId, ClassDataIO::LF_ICON);
		classImages.push_back(prev);
	}

	void drawClassifierWindow()
	{
		if (!ImGui::Begin("Classifier", &show))
		{
			ImGui::End();
			return;
		}
		ImGui::InputText("Name", buffer, 200);

		if (ImGui::Button("Add"))
		{
			BackString st(buffer);
			int classId = proj->addClassType(st);
			classesLB.add(st, {classId, (int)classImages.size()});
			classesLB.endAdding();

			classImages.push_back(ClassPreview());
		}
		const bool hasClasses = classesLB.getSize() != 0;
		ImGui::BeginDisabled(!hasClasses);

		ImGui::SameLine();
		if (ImGui::Button("Edit"))
		{
			BackString st(buffer);
			int selectedClass = classesLB.currentValue().classId;
			classesLB.updateName(classesLB.currentIndex, st);
			proj->changeClassName(selectedClass, st);
		}

		ImGui::SameLine();
		if (ImGui::Button("Drop"))
		{
			auto selectedClass = classesLB.currentValue();
			proj->removeClassType(selectedClass.classId);
			classImages.erase(classImages.begin() + selectedPrevied.vecId);
		}
		ImGui::EndDisabled();

		classesLB.drawListBox("Категории");

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
			ImGui::SameLine();
			if (ImGui::Button("Add"))
			{
				BackImage icon;
				auto selectedClass = classesLB.currentValue();
				size_t dbLocalId = proj->addTrainData(selectedClass.classId, selceted, &icon);
				classImages[selectedClass.vecId].imgs.push_back(TrainPiecePreview(icon, dbLocalId));
			}

			ImGui::SameLine();
			if (ImGui::Button("Drop"))
			{
				auto selectedClass = classesLB.currentValue();
				proj->removeTrainData(selectedClass.classId, selectedPrevied.dbId);
				auto& b = classImages[selectedClass.vecId].imgs;
				b.erase(b.begin() + selectedPrevied.vecId);
			}

			ImGui::SameLine();
			ImGui::BeginGroup();

			int selHei = getPngSize().y;
			auto selSize = getPngSize() * 1.3;
			int off = selSize.x - getPngSize().y / 2;
			//auto& pngs = getClassImagesData().imgs;
			if (ImGui::BeginListBox("##LayersList"))
			{
				int j = 0;
				int selectedClass = classesLB.currentValue().vecId;
				ClassPreview& prev = classImages[selectedClass];
				for (auto& icon : prev.imgs)
				{
					ImGui::PushID(j);
					auto posBef = ImGui::GetCursorPos();
					bool seled = ImGui::Selectable("", icon.dbLocalId == selectedPrevied.dbId, 0, getPngSize() * 1.3);
					if (seled)
						selectedPrevied = {icon.dbLocalId, j};
					auto posAfter = ImGui::GetCursorPos();
					ImGui::SetCursorPos(posBef + ImVec2(off, off));
					ImGui::Image(icon.img.getTexturePtr(), getPngSize());

					ImGui::SetCursorPos(posAfter);

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
