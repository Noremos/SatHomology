// https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
#include "Application.h"

#include <cmath>
#include <unordered_map>
#include <map>
#include <initializer_list>
#include <memory>

#include "GuiWidgets.h"

#include "presets.h"
#include "frontend.h"

namespace MyApp
{
	void setImGuiStyle(float highDPIscaleFactor)
	{
		ImGuiStyle& style = ImGui::GetStyle();

		// https://github.com/ocornut/imgui/issues/707#issuecomment-415097227
		style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.40f, 0.41f, 0.42f, 1.0f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 0.67f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.70f, 0.70f, 0.70f, 0.31f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.70f, 0.70f, 0.70f, 0.80f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.50f, 0.52f, 1.00f);
		style.Colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.72f, 0.72f, 0.72f, 0.78f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.73f, 0.60f, 0.15f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);

		style.WindowPadding = ImVec2(8, 6);
		style.WindowRounding = 0.0f;
		style.FramePadding = ImVec2(5, 7);
		//style.FrameRounding            = 0.0f;
		style.ItemSpacing = ImVec2(5, 5);
		// style.ItemInnerSpacing         = ImVec2(1, 1);
		// style.TouchExtraPadding        = ImVec2(0, 0);
		// style.IndentSpacing            = 6.0f;
		// style.ScrollbarSize            = 12.0f;
		// style.ScrollbarRounding        = 16.0f;
		// style.GrabMinSize              = 20.0f;
		// style.GrabRounding             = 2.0f;
		// style.WindowTitleAlign.x = 0.50f;
		// style.FrameBorderSize = 0.0f;
		// style.WindowBorderSize = 1.0f;

		style.ScaleAllSizes(highDPIscaleFactor);
	}

	GuiBackend backend;

	// Structs

	struct TopbarValues
	{
		// Component
		SelectableKeyValues<bc::ComponentType> componentCB =
		{
			{bc::ComponentType::Component, "Компонента"},
			{bc::ComponentType::Hole, "Дыра"}
		};
		// ---

		// Proc Type
		SelectableKeyValues<bc::ProcType> procCB =
		{
			{bc::ProcType::f0t255, "От 0 до 255"},
			{bc::ProcType::f255t0, "От 0 до 255"},
			{bc::ProcType::Radius, "По расстоянию"},
			{bc::ProcType::invertf0, "Инвертировать"},
			{bc::ProcType::experiment, "Радар"},
			{bc::ProcType::ValueRadius, "Тру растояние"}
		};

		SelectableKeyValues<bc::ColorType> colorCB =
		{
			{bc::ColorType::native, "Как в изображении"},
			{bc::ColorType::gray, "Серый"},
			{bc::ColorType::rgb, "Цветной"},
		};

		GuiTilePreview tilePrview;

		bool enableProcessBtn = false;
		int newTileSize = 20;
		int newOffsetSize = 10;

		int getTileSize()
		{
			return newTileSize * 10; //step
		}
		int getOffsetSize()
		{
			return newOffsetSize * 10; //step
		}

		ImVec2 getImageSize()
		{
			const auto p = backend.getImageSize();
			return ImVec2(p.wid, p.hei);
		}

		FilterInfo filterInfo;
	};

	TopbarValues tbVals;

	// Center

	struct ImagesValues
	{
		GuiDrawImage mainImage;
		GuiDrawImage processImage;
	};

	ImagesValues centerVals;

	struct BottomBar
	{
		int classId = 0;
		std::string valeExtra = "barclass;";
		bool showLoader = false;
	};
	BottomBar bottomVals;


	void loaderCategors(int classId, const BackString& name);
	void loaderFile(int classId, const BackPathStr& path);

	struct ClassiferVals
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
		int classIndex = 0;
		int itemCurrent = 0;

		int getClassImagesSize()
		{
			return classImages.size();
		}
		ClassTextures& getClassImagesData()
		{
			return classImages[classIndex];
		}

		ImVec2 getPngSize()
		{
			return ImVec2(50, 50);
		}
		
		void loadClassImages()
		{
			std::function<void(int, const BackString&)> casFS = loaderCategors;
			std::function<void(int, const BackPathStr&)> casif = loaderFile;

			BarClassifierCache bcc;
			bcc.loadCategories(casFS);
			bcc.loadImgs(casif, classesLB.getValuesIterator(), classesLB.getSize());
		}
	};
	ClassiferVals classerVals;

	void loaderCategors(int classId, const BackString& name)
	{
		classerVals.classesLB.add(name, classId);
	}

	void loaderFile(int classId, const BackPathStr& path)
	{
		BackImage a = imread(path);
		a.resize(classerVals.getPngSize().x, classerVals.getPngSize().y);
		GuiImage img;
		img.setImage(a);
		classerVals.classImages[classId].imgs.push_back(img);
	}
	// Draws

	// Top Menu
	void drawMenu()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::MenuItem("File")) {
				// Do something when File is clicked
			}
			if (ImGui::MenuItem("Edit")) {
				// Do something when Edit is clicked
			}
			//ImGui::Separator();
			if (ImGui::MenuItem("View")) {
				// Do something when View is clicked
			}

			if (ImGui::MenuItem("Item 2")) {
				// Do something when Item 2 is clicked
			}

			ImGui::EndMainMenuBar();
		}
	}
	// --------

	
	// Top bar


	void drawTopBar()
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
		float heighto = ImGui::GetFrameHeight();
		if (ImGui::BeginViewportSideBar("##TopMenu", NULL, ImGuiDir_Up, 50, window_flags))
		{
			//ImGui::Text("Happy secondary menu bar");

			ImGui::SameLine();

			if (ImGui::Button("Load"))
			{
				BackPathStr path = openImageOrProject();
				if (!path.empty())
				{
					backend.loadImageOrProject(path);
					if (backend.isImageLoaded())
					{
						tbVals.enableProcessBtn = true;
						centerVals.mainImage.tileSize = backend.getTileSize();
						classerVals.loadClassImages();
					}
				}
			}
			
			ImGui::SameLine();
			tbVals.componentCB.drawCombobox("##Форма");
			ImGui::SameLine();
			tbVals.procCB.drawCombobox("##Обработка");
			ImGui::SameLine();
			tbVals.colorCB.drawCombobox("##Цвет");

	
			// Always center this window when appearing
			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			ImGui::SameLine();
			ImGui::BeginDisabled(!tbVals.enableProcessBtn);
			if (ImGui::Button("Настроки"))
			{
				tbVals.newTileSize = backend.getTileSize() / 10;
				tbVals.newOffsetSize = backend.getOffsetSize() / 10;
				ImGui::OpenPopup("ProcSetts");
			}

			if (ImGui::BeginPopupModal("ProcSetts", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Порог отсеивания");
				ImGui::SliderInt("##Prog", &tbVals.filterInfo.minLen, 1, 256, "%d");

				ImGui::Separator();

				ImGui::Text("Tile size");
				ImGui::SliderInt("##Tile size", &tbVals.newTileSize, 1, backend.getImageMinSize() / 10, "%d0");
				if (tbVals.newTileSize + tbVals.newOffsetSize > backend.getImageMinSize() / 10)
					tbVals.newOffsetSize = backend.getImageMinSize() / 10 - tbVals.newTileSize;

				ImGui::Text("##Tile offset size");
				ImGui::SliderInt("Offset size", &tbVals.newOffsetSize, 0, backend.getImageMinSize() / 10 - tbVals.newTileSize, "%d0");

				ImGui::Separator();
				auto id = ImGui::FindWindowByName("ProcSetts")->ID;
				tbVals.tilePrview.draw(id, tbVals.getTileSize(), tbVals.getOffsetSize(), tbVals.getImageSize());
				//if (ImGui::IsItemHovered())
				//	ImGui::SetTooltip("I am a tooltip over a popup");

				//static int unused_i = 0;
				//ImGui::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

				ImGui::Separator();
				if (ImGui::Button("OK", ImVec2(120, 0)))
				{ 
					ImGui::CloseCurrentPopup(); 
					backend.getTileSize() = tbVals.getTileSize();
					backend.getOffsetSize() = tbVals.getOffsetSize();
					centerVals.mainImage.tileSize = tbVals.getTileSize();
				}

				ImGui::SetItemDefaultFocus();
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}

			ImGui::SameLine();
			if (ImGui::Button("Построить баркод")) 
			{
				
				backend.createBarcode(
					tbVals.procCB.currentValue(),
					tbVals.colorCB.currentValue(),
					tbVals.componentCB.currentValue(), tbVals.filterInfo);
			}
			ImGui::EndDisabled();

			ImGui::SameLine();
			if (ImGui::Button("Восстановить")) {
				backend.restoreSource();
			}
			ImGui::End();
		}
	}

	// ------

	// How to do a toolbar in Dear ImGui.

	const float toolbarSize = 50;
	int menuBarHeight;

	//// Get the size of the image
	//int width = 512; // Replace this with your own width value
	//int height = 512; // Replace this with your own height value


	// Workout widght


	void drawWorkout()
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		
		ImVec2 pos = viewport->WorkPos;
		ImVec2 size = viewport->WorkSize;
		size.x /= 2;

		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);
		ImGui::SetNextWindowViewport(viewport->ID);
		centerVals.mainImage.drawImage("Main");

		pos.x += size.x;
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);
		ImGui::SetNextWindowViewport(viewport->ID);
		centerVals.processImage.drawImage("Processed");
	}
	// ------

	// Bootom bar
	void drawBottomBar()
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
		// https://github.com/bkaradzic/bgfx/blob/master/3rdparty/dear-imgui/widgets/range_slider.inl
		if (ImGui::BeginViewportSideBar("##BottomBar", NULL, ImGuiDir_Down, 100, window_flags))
		{
			//ImGui::Text("Happy secondary menu bar");
			ImGui::SameLine();
			if (ImGui::Button("Update"))
			{
				backend.processMain(bottomVals.valeExtra);
				// Do something when Button 1 is clicked
			}

			ImGui::SameLine();
			if (ImGui::Button("Add class"))
			{
				backend.addClass(bottomVals.classId);
				// Do something when Button 1 is clicked
			}

			ImGui::SameLine(0, 30);
			ImGui::SetNextItemWidth(100);
			ImGui::InputInt("##input class", &bottomVals.classId, 1, 100);
			if (bottomVals.classId < 0)
				bottomVals.classId = 0;

			ImGui::SameLine(0, 30);
			if (ImGui::Button("Undo"))
			{
				backend.undoAddClass();
			}

			ImGui::SameLine();
			if (ImGui::Button("Выгрузить")) {
				// Open a file dialog to select a folder
				backend.exportResult(getSavePath({ "*.png" }));
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(300);
			ImGui::InputText("Extra", (char*)bottomVals.valeExtra.c_str(), bottomVals.valeExtra.length());

			ImGui::SameLine(0, 30);
			if (ImGui::Button("Loader"))
			{
				bottomVals.showLoader = true;
			}
		}
		ImGui::End();
	}


	//void drawProcessSettings()
	//{
	//}


	void drawClassifierMenu()
	{
		if (ImGui::Begin("Classifier"))
		{
			classerVals.classesLB.drawListBox("Классы");
			//ImGui::ListBox("My List Box", &item_current, items, IM_ARRAYSIZE(items), 4);
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
				tbVals.tilePrview.draw(id, tbVals.getTileSize(), tbVals.getOffsetSize(), tbVals.getImageSize());
				//if (ImGui::IsItemHovered())
				//	ImGui::SetTooltip("I am a tooltip over a popup");

				//static int unused_i = 0;
				//ImGui::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

				ImGui::Separator();
				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					ImGui::CloseCurrentPopup();
					backend.getTileSize() = tbVals.getTileSize();
					backend.getOffsetSize() = tbVals.getOffsetSize();
					centerVals.mainImage.tileSize = tbVals.getTileSize();
				}

				ImGui::SetItemDefaultFocus();
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}


			if (ImGui::Button("Drop")) {
				// Open a file dialog to select a folder
				backend.exportResult(getSavePath({ "*.png" }));
			}
			ImGui::BeginGroup();

			auto& pngs = classerVals.getClassImagesData().imgs;
			for (int i = 0; i < classerVals.getClassImagesSize(); i++)
			{
				ImGui::PushID(i);
				ImGui::Image((void*)(intptr_t)pngs[i].textureId, classerVals.getPngSize());
				ImGui::PopID();
			}

			ImGui::EndGroup();

		}
		ImGui::End();
	}

	
	// Layout
	void drawLayout()
	{
		drawTopBar();
		drawWorkout();
		drawBottomBar();
		if (bottomVals.showLoader)
		{
			drawClassifierMenu();
		}

		//if (ImGui::BeginViewportSideBar("##MainStatusBar", NULL, ImGuiDir_Down, heighto, window_flags)) {
		//    if (ImGui::BeginMenuBar()) {
		//        ImGui::Text("Happy status bar");
		//        ImGui::EndMenuBar();
		//    }
		//}
		//ImGui::End();
		//
		// !!!!!!!!!!!!!!!!!!!!!!!!!!1
	}


	void MyApp::Init()
	{
		backend.settup(&centerVals.mainImage, &centerVals.processImage, NULL);
	}
	// Main
	void MyApp::RenderUI()
	{
		ImGuiID dockspace_id = 0;
		ImGuiWindowFlags window_flags = 0;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		bool opt_fullscreen = true;
		bool opt_padding = false;
		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_AutoHideTabBar;

		window_flags = 0;

		ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
		ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);


		drawLayout();

		//ImGui::DockSpace(ImGui::GetID("MyDockspace"), ImVec2(0, 0),
		//    ImGuiDockNodeFlags_NoDockingInCentralNode);

		// Display the image in imgui
		window_flags = 0;
		window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

		ImGuiWindowClass window_class;
		window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
		//ImGui::SetNextWindowClass(&window_class);
		//window_flags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		//window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;


		//ImGui::SetNextWindowClass(&window_class);


		/*
		ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
		ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);
		//ImGuiID dock_id_prop = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);
		ImGuiID dock_id_up = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.10f, NULL, &dock_main_id);
		ImGuiID dock_id_down = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, NULL, &dock_main_id);

		//ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.50f, NULL, &dock_main_id);
		//ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.50f, NULL, &dock_main_id);
		//ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.10f, NULL, &dock_main_id);
		ImGui::DockBuilderDockWindow("Top Menu", dock_id_up);
		ImGui::DockBuilderDockWindow("Down Menu", dock_id_down);
		//ImGui::DockBuilderDockWindow("Photo 1", dock_id_left);
		//ImGui::DockBuilderDockWindow("Photo 2", dock_id_right);
		ImGui::DockBuilderFinish(dockspace_id);

		//ImGui::DockBuilderRemoveNode(imgId); // clear any previous layout
		//ImGui::DockBuilderAddNode(imgId, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
		//ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

		//ImGui::DockBuilderDockWindow("Properties", dock_id_prop);
		//ImGui::DockBuilderDockWindow("Mesh", dock_id_prop);
		//ImGui::DockBuilderDockWindow("Extra", dock_id_prop);

		*/

		//ImGui::DockBuilderDockWindow("Photo 1", 0);


		ImGui::ShowDemoWindow();
	}
}
