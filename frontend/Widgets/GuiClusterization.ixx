module;
#include "../GuiCommon.h"

export module GuiClusterization;

import ProjectModule;
import IOCore;
import Platform;
import GuiWidgets;
import TrainIO;
import ClusterInterface;
import TreeClassifier;
import RasterBarHolderRLayer;

struct StringBuffer
{
	char buffer[200];

	StringBuffer()
	{
		memset(buffer, 0, 200);
	}

	StringBuffer operator=(std::string_view t)
	{
		memcpy(buffer, t.data(), t.length());
		return *this;
	}

	bool hasData() const
	{
		return strlen(buffer) > 0;
	}
};

//
//
export class GuiClusterizationWindow
{
	std::unique_ptr<IBarClusterizer> clusterizer;
	BarCategories clusterCategs;

public:

	struct GuiClusterMethod
	{
		int methodId;
		BackString name;
		GuiClusterMethod(int id, const BackString& name) : methodId(id), name(name)
		{ }
	};

	struct GuiClass
	{
		int classId;
		GuiClass(int id) : classId(id)
		{ }
	};

	SelectableKeyValues<GuiClusterMethod> clsuterMethods;
	SelectableKeyValues<GuiClass> classesLB;

	bool show = false;
	//InOutLayer* ioLayer;

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
	GuiClusterizationWindow() : clusterizer(nullptr)
	{
		curColor = ImVec4(1.0, 1.f, 1.f, 1.f);
		//c;

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
		//clusterCategs

	}

	RasterLineLayer* line;
	BackString nameBuffer;
	void init(RasterLineLayer* line)
	{
		//classes.clear();
		//clusterizer->loadData();

		//for (auto&& c : proj->classCategs.categs)
		//{/
		//	classesLB.add(c.name, { c.id });
		//	loadClassImages(io, classesLB.back());
		//}
		//classesLB.endAdding();

		//if (classesLB.getSize() > 0)
		//	setCurrentClassName();

		auto& factory = getClusterFactory();
		for (int i = 0; i < factory.getCount(); i++)
		{
			auto& name = factory.getNames()[i];
			clsuterMethods.add(name, { i, name });
		}
		clsuterMethods.endAdding();
		this->line = line;
	}

	BackColor& getCurColor()
	{
		int claId = classesLB.currentValue().classId;
		return clusterCategs.get(claId)->color;
	}

	void draw()
	{
		if (!show)
			return;

		if (!ImGui::Begin("Кластеризация", &show))
		{
			ImGui::End();
			return;
		}
		clsuterMethods.drawCombobox("Метод кластеризации");
		//clsuterMethods.

		char nameCharBuffer[200]{};
		memcpy(nameCharBuffer, nameBuffer.c_str(), nameBuffer.length());
		nameCharBuffer[nameBuffer.length()] = '\0';
		ImGui::InputText("Имя", (char*)nameCharBuffer, 200);
		nameBuffer = nameCharBuffer;

		const bool emptyName = nameBuffer.length() == 0;
		ImGui::BeginDisabled(emptyName);
		if (ImGui::Button("Добавить"))
		{
			const int classId = clusterCategs.addValue(nameBuffer);

			classesLB.add(nameBuffer, { classId });
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
			int selectedClass = classesLB.currentValue().classId;
			classesLB.updateName(classesLB.currentIndex, nameBuffer);
			clusterCategs.changeName(classesLB.currentIndex, nameBuffer);
			//proj->changeClassName(selectedClass, nameBuffer);
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Удалить"))
		{
			int selClassID = classesLB.currentValue().classId;
			classesLB.remove(classesLB.currentIndex);
			clusterCategs.remove(selClassID);
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
			//	graph.init(proj->classifier.dbPath, classId);
			//	graphBarlines.initBarlines(proj->classifier.dbPath, classId);
			//}

			//ImGui::BeginDisabled(!selceted.hasData());

			if (ImGui::Button("Запустить"))
			{
				auto methodId = clsuterMethods.currentValue().methodId;
				clusterizer = getClusterFactory().CreateML(methodId);
				line->collectionToPredict = getClusterFactory().CreateHolder(methodId);
				line->processCachedBarcode(nullptr, false);
				clusterizer->setClassesCount(classesLB.getSize());
				if (line->collectionToPredict->getItemsCount() == 0)
				{
					ImGui::OpenPopup("NoDataToCluster");
				}
				else
				{
					clusterizer->predict(*line->collectionToPredict);
				}
			}

			if (ImGui::BeginPopupModal("NoDataToCluster", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("There are no data to cluster");

				if (ImGui::Button("ОК"))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			//ImGui::EndDisabled();



			ImGui::BeginGroup();

			// int selHei = getPngSize().y;
			auto selSize = getPngSize() * 1.3;
			float off = (selSize.x - getPngSize().x) / 2;
			//auto& pngs = getClassImagesData().imgs;

			float winWid = ImGui::GetCurrentWindow()->Size.x;

			//if (ImGui::BeginListBox("##ClassDataList", ImVec2(winWid, 0)))
			//{
			//	ImVec2 posAfter;

			//	int j = 0;
			//	auto& prevImgs = classesLB.currentValue().imgs;

			//	float stX = 0;
			//	for (auto& icon : prevImgs)
			//	{
			//		ImGui::PushID(j);

			//		ImVec2 posBef;
			//		if (j != 0)
			//		{
			//			ImGui::SetCursorPos(posAfter);
			//			posBef = posAfter;
			//		}
			//		else
			//		{
			//			posBef = ImGui::GetCursorPos();
			//			stX = posBef.x;
			//		}

			//		bool seled = ImGui::Selectable("", icon.dbLocalId == selectedPrevied.dbId, 0, selSize);
			//		if (seled)
			//			selectedPrevied = { icon.dbLocalId, j };

			//		ImGui::SetCursorPos(posBef + ImVec2(off, off));
			//		ImGui::Image(icon.img.getTexturePtr(), getPngSize());

			//		float newXoff = posBef.x + selSize.x + 3;
			//		if (newXoff >= winWid)
			//		{
			//			posAfter = { stX, posAfter.y + selSize.y };
			//		}
			//		else
			//			posAfter = { newXoff, posBef.y };

			//		//ImGui::SetCursorPos(posAfter);

			//		//ImGui::SameLine();

			//		ImGui::PopID();
			//		++j;
			//	}
			//	ImGui::EndListBox();
			//}
			ImGui::EndGroup();
		}
		ImGui::End();
	}
};
