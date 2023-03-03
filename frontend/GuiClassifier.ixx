module;
#include "GuiCommon.h"

export module GuiClassifierModule;

import ProjectModule;
import IOCore;
import Platform;
import GuiWidgets;

Project* proj = Project::getProject();

export struct GuiClassifer
{
	SelectableKeyValues<int> classesLB;

	struct ClassTextures
	{
		int id;
		std::vector<GuiImage> imgs;
		ClassTextures(int i) : id(i)
		{ }
	};

	GuiDrawImage iconImage;
	std::vector<ClassTextures> classImages;
	int itemCurrent = 0;
	bool show = false;

	int getClassImagesSize()
	{
		return classImages.size();
	}
	ClassTextures& getClassImagesData()
	{
		return classImages[classesLB.currentValue()];
	}

	ImVec2 getPngSize()
	{
		return ImVec2(32, 32);
	}

	void loaderCategors(int classId, const BackString& name)
	{
		classesLB.add(name, classId);
		classImages.push_back(ClassTextures(classId));
	}

	void loaderFile(int classId, const BackPathStr& path)
	{
		BackImage a = imread(path);
		if (a.wid() == 0)
			throw;
		int wid = a.wid();
		int hei = a.hei();
		auto sa = getPngSize();
		ResizeImage(wid, hei, sa.x, sa.y);
		a.resize(wid, hei);

		GuiImage img;
		img.setImage(a);
		classImages[classId].imgs.push_back(std::move(img));
	}

	void loadClassImages()
	{
		//std::function<void(int, const BackString&)> casFS = loaderCategors;
		//std::function<void(int, const BackPathStr&)> casif = loaderFile;

		/*BarClassifierCache bcc;
		bcc.loadCategories(casFS);
		classesLB.endAdding();
		bcc.loadImgs(casif, classesLB.getValuesIterator(), classesLB.getSize());*/
	}


	void drawClassifierWindow()
	{
		if (!show)
			return;

		if (ImGui::Begin("Classifier"))
		{
			classesLB.drawListBox("������");
			//ImGui::ListBox("My List Box", &item_current, items, IM_ARRAYSIZE(items), 4);


			if (ImGui::Button("Add selected"))
			{
				BackImage icon;
				int selectedClass = classesLB.currentValue();
				//if (backend.addSelectedToClassData(selectedClass, &icon))
				//{
				//	GuiImage img;
				//	img.setImage(icon);
				//	classImages[selectedClass].imgs.push_back(img);
				//}
				//backend.undoAddClass();
			}
			ImGui::SameLine();


			if (ImGui::Button("Drop"))
			{
				// Open a file dialog to select a folder
				//backend.exportResult(getSavePath({ "*.png" }));
			}

			ImGui::SameLine();

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
				auto id = ImGui::FindWindowByName("ProcSetts")->ID;
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

			ImGui::BeginGroup();

			//auto& pngs = getClassImagesData().imgs;

			//int curItem = 0;
			//auto imgLoader = [](void* data, int idx, const char** out_text) {
			//		auto& pngs = *static_cast<std::vector<GuiImage>*>(data);
			//		*out_text = pngs[idx].name.c_str();
			//		return true;
			//	};
			//ImGui::ListBox("Images", &curItem, imgLoader, &pngs, pngs.size());

			//for (size_t j = 0; j < pngs.size(); j++)
			//{
			//	ImGui::PushID(j);
			//	//ImGui::Image(pngs[j].getTexturePtr(), ImVec2(pngs[j].width, pngs[j].height));
			//	ImGui::PopID();
			//}
			ImGui::EndGroup();

		}
		ImGui::End();
	}
};
