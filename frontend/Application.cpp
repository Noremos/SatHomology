// https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
#include "Application.h"
#include "DrawCommon.h"

#include <cmath>
#include <initializer_list>
#include <memory>
#include <future>
#include <iostream>
#include "../side/implot/implot.h"
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "Json.h"

//#include "sol3/sol.hpp"
//
//#include <GLFW/glfw3.h>
//#include "../libs/glew/include/GL/glew.h"
#include "../../backend/ProjectSettings.h"
#include "../Bind/Framework.h"

#include "../backend/MatrImg.h"
#include "../backend/Layers/layerInterface.h"
#include "../backend/Layers/Rasterlayers.h"
#include "../backend/Layers/RasterLineLayer.h"

import FrontendBind;
// import Platform;
import GuiLayers;
// import BarcodeModule;
import GuiOverlap;
import GuiClassifierModule;
import GuiWidgets;
import VectorLayers;
import GuiVectorLayers;
import ProjectModule;
import GuiRasterLayers;
import GuiRasterLineLayer;
// import ProjectSettings;
//import BackBind;
// import RasterLineLayerModule;

import DrawUtils;
// import RasterLayers;
// import LayersCore;
import IGuiLayer;
import GuiLayers;
import IAlgorithm;
import GuiBlock;
//import Lua;



/// Widget for raster layers

GuiBackend backend;
int maxThreadCount, minThreadCount;




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
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

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

	//template<class T>
	//struct ValBinder
	//{
	//	T* from;
	//	T* to;
	//};

	GuiClassifer classerVals;
	bool useAsync = false;// true;
	// Structs

	struct
	{
		std::mutex drawMutex;  // protects g_i
		std::vector<bc::point> debugDraw;
		std::vector<bc::PloyPoints> debugPlygon;
		std::vector<std::pair<ImVec2, ImVec2>> debugLine;
	} debugVals;


	struct WindowsValues
	{
		bool onAir = false;

		void onAirC()
		{
			if (onAir && future.valid())
			{
				if (future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
				{
					onAir = false;
				}
			}
		}

		std::future<void> future;
	};

	WindowsValues commonValus;
	LayersVals layersVals;// (backend);

	struct TopbarValues
	{
		bool enableProcessBtn = false;
		ProjectionSettings projset;
	};

	TopbarValues tbVals;

	// Center

	struct ImagesValues
	{
		GuiCSDisplayContainer resizble;
		//GuiResizableContainer resizble;

		// vecotr<IOverlap*>
	};

	ImagesValues centerVals;

	struct BottomBar
	{
		int classId = 0;
		std::string valeExtra = "barclass;";
		BackString debug;
		bool drawPics = true;
		bool showUpdaePopup = false;
	};
	BottomBar bottomVals;

	// -------------------

	// Draws

	// Top Menu
	// void drawMenu()
	// {
	// 	if (ImGui::BeginMainMenuBar())
	// 	{
	// 		if (ImGui::MenuItem("File")) {
	// 			// Do something when File is clicked
	// 		}
	// 		if (ImGui::MenuItem("Edit")) {
	// 			// Do something when Edit is clicked
	// 		}
	// 		//ImGui::Separator();
	// 		if (ImGui::MenuItem("View")) {
	// 			// Do something when View is clicked
	// 		}

	// 		if (ImGui::MenuItem("Item 2")) {
	// 			// Do something when Item 2 is clicked
	// 		}

	// 		ImGui::EndMainMenuBar();
	// 	}
	// }
	// --------


	// Top bar
	int tempThreads;
	bool runAsync;

	void drawProjectSettings()
	{
		if (ImGui::Button(BU8("Project settings")))
		{
			ImGui::OpenPopup("ProjectSetts");
			tempThreads = getSettings().threadsCount;
			runAsync = getSettings().runAsync;
		}

		if (ImGui::BeginPopupModal("ProjectSetts", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::BeginDisabled(minThreadCount < 2);
			ImGui::Checkbox(BU8("Async"), &runAsync);
			ImGui::EndDisabled();

			if (minThreadCount > 1)
			{
				ImGui::SetNextItemWidth(150);

				ImGui::BeginDisabled(!runAsync);
				ImGui::InputInt("##thr", &tempThreads, 2, maxThreadCount);
				ImGui::EndDisabled();
			}

			ImGui::Separator();
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				getSettings().threadsCount = tempThreads;
				getSettings().runAsync = runAsync;

				ImGui::CloseCurrentPopup();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button(BU8("Cancel"), ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void drawLayerSettings()
	{
		if (ImGui::Button(BU8("Layer properties")))
		{
			ImGui::OpenPopup("ProcSetts");
		}

		if (ImGui::BeginPopupModal("ProcSetts", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			layersVals.getCurrentLayer()->drawProperty();
			//if (ImGui::IsItemHovered())
			//	ImGui::SetTooltip("I am a tooltip over a popup");

			//static int unused_i = 0;
			//ImGui::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

			ImGui::Separator();
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				layersVals.getCurrentLayer()->applyPropertyChanges();

				// IRasterLayer* core = layersVals.getCurrentRasterCore();

				// if (core)
					// centerVals.tilemap.setTilesize(core.prov.tileSize);
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button(BU8("Отмена"), ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::EndDisabled();
	}

	void drawTopBar()
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;
		// float heighto = ImGui::GetFrameHeight();
		if (ImGui::BeginViewportSideBar("##TopMenu", NULL, ImGuiDir_Up, 50, window_flags))
		{
			// GBl
			ImGui::BeginDisabled(commonValus.onAir);

			// if (ImGui::Button("Create prj"))
			// {
			// 	ImGui::OpenPopup("CreateProject");
			// }

			if (ImGui::BeginPopupModal("CreateProject", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				static char str0[128] = "Hello, world!";
				ImGui::InputText("Name", str0, IM_ARRAYSIZE(str0));

				static char path[1024] = "";

				ImGui::InputText("Path", path, IM_ARRAYSIZE(str0));

				//if (ImGui::Button("Select proj"))
				//{
				//	BackPathStr pathi = openDir();
				//	memcpy(path, pathi.string().c_str(), pathi.string().length());
				//}

				if (ImGui::Button("Select img"))
				{
					BackPathStr pathi = openImage();
					memcpy(path, pathi.string().c_str(), pathi.string().length());
				}

				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					[[maybe_unused]]
					auto* core = backend.loadImageOrProject(path);
					if (backend.isLoaded())
					{

						//centerVals.tilemap.init(tileSize = backend.getTileSize();
						//classerVals.loadClassImages();
					}

					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();
				if (ImGui::Button(BU8("Отмена"), ImVec2(120, 0)))
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			static std::queue<RasterFromDiskLayer*> layers;

			ImGui::SameLine();
			if (ImGui::Button(BU8("Open...")))
			{
				std::vector<BackPathStr> paths = openImagesOrProject();
				for (auto& path : paths)
				{
					RasterFromDiskLayer* layer = backend.loadImageOrProject(path);
					if (backend.isLoaded())
					{
						if (layer)
						{
							if (layer->hasCS())
							{
								tbVals.enableProcessBtn = true;
								auto* guiLayer = layersVals.addLayer<RasterFromDiskGuiLayer>("Loaded", layer);
								guiLayer->lockAtThis(layersVals.lastRealSize);
							}
							else
							{
								layer->cs.init(DEFAULT_PROJECTION);
								tbVals.projset.setup(layer->cs);
								ImGui::OpenPopup(BU8("Coord System"));
								layers.push(layer);
							}
						}
						else
						{
							tbVals.enableProcessBtn = true;
							layersVals.loadLayers();
						}
						//layer->data = backend.getMain();
						// centerVals.heimap.init(layer->getData()->mat);
						// centerVals.tilemap.init(&layer->main, backend.getTileSize());
						//classerVals.init();
					}
				}
			}

			if (ImGui::BeginPopupModal(BU8("Coord System"), NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				//tbVals.projset.draw();
				while (!layers.empty())
				{
					ImGui::Separator();
					//if (ImGui::Button("OK", ImVec2(120, 0)))
					{
						auto* layer = layers.front();
						tbVals.projset.apply(layer->cs);
						layer->cache(backend.getMeta());

						auto* guiLayer = layersVals.addLayer<RasterFromDiskGuiLayer>("Loaded", layer);
						guiLayer->lockAtThis(layersVals.lastRealSize);
						layers.pop();

						tbVals.enableProcessBtn = true;
						ImGui::CloseCurrentPopup();
					}

					ImGui::SameLine();
					if (ImGui::Button(BU8("Cancel"), ImVec2(120, 0)))
					{
						backend.removeLayer(layers.front()->id);
						layers.pop();

						ImGui::CloseCurrentPopup();
					}
				}

				ImGui::EndPopup();
			}

			// Always center this window when appearing
			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			ImGui::SameLine();
			ImGui::BeginDisabled(!tbVals.enableProcessBtn);


			ImGui::SameLine();
			drawProjectSettings();


			// ----------------------------
			ImGui::SameLine();
			drawLayerSettings();

			// ---------------------------------
			ImGui::SameLine();
			//if (ImGui::Button("Восстановить"))
			//{
			//}

			ImGui::SameLine();
			if (ImGui::Button(BU8("Save")))
			{
				backend.save();
			}

			// GBl
			ImGui::EndDisabled();

			ImGui::End();
		}
	}

	// ------

	// How to do a toolbar in Dear ImGui.

	// const float toolbarSize = 50;
	int menuBarHeight;

	//// Get the size of the image
	//int width = 512; // Replace this with your own width value
	//int height = 512; // Replace this with your own height value


	// Workout width


	void drawWorkout()
	{
		//// First collapsible section
		//if (ImGui::CollapsingHeader("Section 1"))
		//{
		//	// Content of first section
		//	ImGui::Text("Hello from Section 1!");
		//}

		//// Second collapsible section
		//if (ImGui::CollapsingHeader("Section 2"))
		//{
		//	// Content of second section
		//	ImGui::Text("Hello from Section 2!");
		//}
		//ImGui::EndChild();
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 pos = viewport->WorkPos;
		ImVec2 drawSize = viewport->WorkSize;

		//ImGui::SetNextWindowViewport(viewport->ID);

		auto window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking;
			window_flags |= ImGuiWindowFlags_HorizontalScrollbar;

		GuiDisplaySystem guiDisplay(backend.getDS());

		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(drawSize);

		guiDisplay.drawPos = {0,0};
		guiDisplay.drawSize = BackPoint(drawSize.x, drawSize.y);
		guiDisplay.cursorPos = centerVals.resizble.currentPos;
		if (centerVals.resizble.Begin("ImagePreview"))
		{
			layersVals.draw(guiDisplay);
			if (centerVals.resizble.clicked)
			{
				layersVals.onClick(guiDisplay, centerVals.resizble.clickedPos);
				RasterLineGuiLayer* lay = layersVals.getCastCurrentLayer<RasterLineGuiLayer>();
				if (lay && lay->selectedLine)
				{
					classerVals.selceted = { lay->selectedLine->tileId, lay->selectedLine->barlineIndex};
				}
			}

			// TiledRasterGuiLayer<RasterFromDiskLayer>* tlay = layersVals.getCastCurrentLayer<TiledRasterGuiLayer<RasterFromDiskLayer>>();
			layersVals.drawOverlap(guiDisplay);
		}

		centerVals.resizble.end(guiDisplay.getWinPos(), guiDisplay.getDrawSize());


		pos.x += drawSize.x;
		// drawSize.x = 200;
		drawSize.y += 20;

		// ImGui::SetNextWindowPos(pos);
		// ImGui::SetNextWindowSize(drawSize);
		//ImGui::SetNextWindowViewport(viewport->ID);

		window_flags = 0;//ImGuiWindowFlags_NoTitleBar;// | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar;
		if (ImGui::BeginViewportSideBar("Sidebar", NULL, ImGuiDir_Right, 200, window_flags))
		{
		// if (ImGui::Begin("Sidebar", NULL, window_flags))

			ImGui::End();
		}

		//
		//pos.x += size.x;
		//size.x = leftProps;
		//ImGui::SetNextWindowPos(pos);
		//ImGui::SetNextWindowSize(size);
		////ImGui::SetNextWindowViewport(viewport->ID);
		//if (ImGui::Begin("properties", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus))
		//{
		//	leftProps = 200;
		//	//if (ImGui::CollapsingHeader("Layers"))
		//	//{
		//	//	drawLayout();
		//	//}
		//}
		//else
		//	leftProps = 20;
		//ImGui::End();
	}
	// ------

	// Bootom bar
	void drawBottomBar()
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;
		// https://github.com/bkaradzic/bgfx/blob/master/3rdparty/dear-imgui/widgets/range_slider.inl
		if (ImGui::BeginViewportSideBar("##BottomBar", NULL, ImGuiDir_Down, 50, window_flags))
		{
			ImGui::SameLine(0, 30);
			static GuiBlock guiblock;

			if (ImGui::Button("Блоки"))
			{
				guiblock.show = true;
			}
			guiblock.draw(layersVals);

			// GBL
			ImGui::BeginDisabled(commonValus.onAir || !tbVals.enableProcessBtn);

			// ImGui::SameLine();
			// if (ImGui::Button("Выгрузить")) {
			// 	// Open a file dialog to select a folder
			// 	backend.exportResult(getSavePath({ "*.png" }));
			// }

			// ImGui::SameLine();
			// if (ImGui::Checkbox("Enable pics", &bottomVals.drawPics))
			// {
			// 	backend.showResultPics(bottomVals.drawPics);
			// }


			// ImGui::Text(bottomVals.debug.c_str());

			// Current cursor pos
			ImGui::SameLine();
			const auto& curpos = centerVals.resizble.currentPos;
			ImGui::Text("%f : %f   | %f", curpos.x, curpos.y, backend.getDS().csScale);

			// Progress bar
			ImGui::SameLine();
			ImGui::ProgressBar(0.0, ImVec2(0.0f, 0.0f));
			ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::Text(BU8("Progressbar"));

			// GBL
			ImGui::EndDisabled();
			ImGui::End();
		}
	}


	void debugWindow()
	{
		/*if (!ImGui::Begin("debug"))
		{
			ImGui::End();
			return;
		}

		ImDrawList* list = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetWindowPos();
		pos.x += 10;
		pos.y += 50;
		const float mul = 4;

		if (commonValus.onAir)
		{
			const std::lock_guard<std::mutex> lock(debugVals.drawMutex);
			for (auto& i : debugVals.debugPlygon)
			{
				std::vector<ImVec2> vec;
				std::transform(i.begin(), i.end(), std::back_inserter(vec), [mul, pos](bc::point& a)
					{
						return ImVec2(a.x, a.y) * mul + pos;
					});

				list->AddConvexPolyFilled(vec.data(), vec.size(), ImColor(255, 10, 10));
			}

			size_t total = debugVals.debugDraw.size();
			for (size_t i = 0; i < total; i += 2)
			{
				ImVec2 p1(debugVals.debugDraw[i].x, debugVals.debugDraw[i].y);
				ImVec2 p2(debugVals.debugDraw[i + 1].x, debugVals.debugDraw[i + 1].y);
				p1 *= mul;
				p2 *= mul;

				p1 += pos;
				p2 += pos;

				list->AddLine(p1, p2, ImColor(255, 255, 182), 2);
				list->AddCircleFilled(p1, 3, ImColor(50, 255, 0));
				list->AddCircleFilled(p2, 3, ImColor(50, 255, 0));
			}

			for (auto& l : debugVals.debugLine)
			{
				list->AddLine(pos + l.first * mul, pos + l.second * mul, ImColor(255, 0, 0));
			}
		}

		ImGui::End();*/
	}

	void draw3d()
	{
		// Define the vertices of the terrain
		//const GLfloat terrainVertices[] = {
		//	// Position              // Texture coordinates
		//	-1.0f, -1.0f, 0.0f,      0.0f, 0.0f,
		//	1.0f, -1.0f, 0.0f,       1.0f, 0.0f,
		//	-1.0f, 1.0f, 0.0f,       0.0f, 1.0f,
		//	1.0f, 1.0f, 0.0f,        1.0f, 1.0f
		//};

		//// Define the indices of the terrain
		//const GLuint terrainIndices[] = {
		//	0, 1, 2,
		//	2, 1, 3
		//};


		//// Define the vertex shader
		//const char* vertexShaderSource =
		//	"#version 330 core\n"
		//	"layout (location = 0) in vec3 aPos;\n"
		//	"layout (location = 1) in vec2 aTexCoord;\n"
		//	"out vec2 TexCoord;\n"
		//	"uniform mat4 model;\n"
		//	"uniform mat4 view;\n"
		//	"uniform mat4 projection;\n"
		//	"void main()\n"
		//	"{\n"
		//	"   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
		//	"   TexCoord = aTexCoord;\n"
		//	"}\0";

		//// Define the fragment shader
		//const char* fragmentShaderSource =
		//	"#version 330 core\n"
		//	"out vec4 FragColor;\n"
		//	"in vec2 TexCoord;\n"
		//	"uniform sampler2D texture1;\n"
		//	"void main()\n"
		//	"{\n"
		//	"   FragColor = texture(texture1, TexCoord);\n"
		//	"}\n\0";

		//glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection);

		//ImGuiIO& io = ImGui::GetIO();
		//int display_w, display_h;
		//GLFWwindow* window = glfwCreateWindow(800, 600, "Terrarian", NULL, NULL);


		//// Create a vertex shader
		//GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		//glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		//glCompileShader(vertexShader);

		//// Check for vertex shader compile errors
		//int success;
		//char infoLog[512];
		//glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		//if (!success) {
		//	glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		//	std::cout << "Vertex shader compile error: " << infoLog << std::endl;
		//}

		//// Create a fragment shader
		//GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		//glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		//glCompileShader(fragmentShader);

		//glfwGetFramebufferSize(window, &display_w, &display_h);
		//glViewport(0, 0, display_w, display_h);
		//GLuint modelTexture;
		//glGenTextures(1, &modelTexture);
		//glBindTexture(GL_TEXTURE_2D, modelTexture);



		//// Define camera position, target, and up direction
		//glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
		//glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
		//glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

		//// Calculate camera direction and right vector
		//glm::vec3 cameraDirection = glm::normalize(cameraTarget - cameraPos);
		//glm::vec3 cameraRight = glm::normalize(glm::cross(cameraDirection, cameraUp));
		//cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection));

		//// Define camera front vector
		//glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

		//// Set up the model, view, and projection matrices
		//glm::mat4 model = glm::mat4(1.0f); // Identity matrix
		//glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		//glm::mat4 projection = glm::perspective(glm::radians(fov), (float)width / (float)height, 0.1f, 100.0f);

		//// Get the location of the uniform variables in the shader program
		//unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
		//unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
		//unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

		//// Pass the matrices to the shader program
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//// Render the 3D model to the texture
		//glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, modelTexture, 0);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//// render 3D model here...
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//// Display the 3D model in an ImGui window
		//ImGui::Begin("3D Model");
		//ImVec2 size(ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
		//ImGui::Image((void*)(intptr_t)modelTexture, size);
		//ImGui::End();
	}

	//LuaLayers lua;
	//char luatext[10000];
	//void drawScript()
	//{
	//	if (!ImGui::Begin("Скрипт"))
	//	{
	//		ImGui::End();
	//		return;
	//	}

	//	if (ImGui::Button("Выполнить"))
	//	{
	//		lua.setScript(luatext);
	//	}

	//	ImGui::InputTextMultiline("Lua", luatext, 10000, ImVec2(500, 300));
	//	ImGui::End();
	//}

	void ToolSetDraw()
	{
		if (!ImGui::Begin("Tools"))
		{
			ImGui::End();
			return;
		}

		if (ImGui::Button("Add vector layer"))
		{
			//backend.
			VectorLayer* layerData = backend.addVectorLayer();
			layersVals.addLayer<VectorGuiLayer, VectorLayer>("vector", layerData);
		}

		// Add ploygon;
		// Add line
		// Move point
		// Move line
		// Drop
		// export
		// import

		ImGui::End();
	}

	// Layout
	void drawLayout()
	{
		debugWindow();

		drawTopBar();
		drawWorkout();
		drawBottomBar();


		ImGui::BeginDisabled(commonValus.onAir || !tbVals.enableProcessBtn);
		//drawScript();
		//ToolSetDraw();
		layersVals.drawToolbox();
		ImGui::EndDisabled();

		classerVals.drawClassifierWindow();
		//drawClassifierMenu();

		// Subs
		layersVals.drawLayersWindow();

		commonValus.onAirC();

		ImPlot::ShowDemoWindow();

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

	constexpr ImVec2 toDVec2(const bc::point& p)
	{
		return ImVec2(static_cast<float>(p.x), static_cast<float>(p.y));
	}

	void Init(const char* root)
	{

		// bc::barstruct constr;
		// constr.createBinaryMasks = true;
		// constr.createGraph = false;
		// constr.attachMode = bc::AttachMode::morePointsEatLow;
		// //constr.attachMode = bc::AttachMode::closer;
		// constr.returnType = bc::ReturnType::barcode2d;

		// constr.addStructure(bc::ProcType::f0t255, bc::ColorType::gray, bc::ComponentType::Component);

		// BackImage imr = imread(BackPathStr("/Users/sam/Edu/datasets/test_dataset/crater/image_27.png"));
		// BackImage img = imr;//.getRect(2,160,15,3);
		// bc::BarcodeCreator creator;
		// std::unique_ptr<bc::Baritem> citem(creator.createBarcode(&img, constr));

		// setlocale(LC_ALL, "ru_ru.utf-8");
		srand(time(NULL));
		Variables::setRoot(root);


		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	   // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	  // Enable Gamepad Controls


		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;		   // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;		 // Enable Multi-Viewport / Platform Windows


		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();
		MyApp::setImGuiStyle(1.0);

		ImFontConfig font_config;
		font_config.OversampleH = 1; //or 2 is the same
		font_config.OversampleV = 1;
		font_config.PixelSnapH = 1;

		static const ImWchar ranges[] =
		{
			0x0020, 0x00FF, // Basic Latin + Latin Supplement
			0x0400, 0x044F, // Cyrillic
			0,
		};

		[[maybe_unused]]
		ImFont* font = io.Fonts->AddFontFromFileTTF(Variables::getDefaultFontPath().string().c_str(), 15.0f, &font_config, ranges);
		IM_ASSERT(font != NULL);

		//setlocale(LC_ALL, "rus");

		float baseFontSize = 20.0f; // 13.0f is the size of the default font. Change to the font size you use.
		float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

		// merge in icons from Font Awesome
		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
		ImFontConfig icons_config;
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		icons_config.GlyphMinAdvanceX = iconFontSize;
		io.Fonts->AddFontFromFileTTF((Variables::getFontsDir() / FONT_ICON_FILE_NAME_FAS).string().c_str(), iconFontSize, &icons_config, icons_ranges);
		// use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid

		// in an imgui window somewhere...
		// outputs a paint brush icon and 'Paint' as a string.
		//setlocale(LC_CTYPE, "rus"); // ����� ������� ��������� ������


		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		//io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;



		maxThreadCount = std::thread::hardware_concurrency();
		if (maxThreadCount < 2)
		{
			minThreadCount = 1;
		}
		else
			minThreadCount = 2;

		backend.getDS().sysProj.init(DEFAULT_PROJECTION);

		LayerFactory::RegisterFactory<RasterGuiLayer, RasterLayer>(RASTER_LAYER_FID);
		LayerFactory::RegisterFactory<RasterLineGuiLayer, RasterLineLayer>(RASTER_LINE_LAYER_FID);
		LayerFactory::RegisterFactory<RasterFromDiskGuiLayer, RasterFromDiskLayer>(RASTER_DISK_LAYER_FID);
		LayerFactory::RegisterFactory<VectorGuiLayer, VectorLayer>(VECTOR_LAYER_FID);
		//LayerFactory::RegisterFactory<VectorGuiLayer, VetorLayer>(VECTOR_LAYER_FID);
		LayerFactory::RegisterFactory<TreeVectorGuiLayer, TreeVectorLayer>(TREE_VECTOR_LAYER_FID);
		LayerFactory::RegisterFactory<ClassVectorGuiLayer, ClassVectorLayer>(TREE_VECTOR_CLASS_LAYER);

		//classerVals.ioLayer = layersVals.getIoLayer();
		[[maybe_unused]]
		auto drawLine = [](const bc::point& p1, const bc::point& p2, bool finale)
		{
			const std::lock_guard<std::mutex> lock(debugVals.drawMutex);

			if (finale)
			{
				debugVals.debugLine.clear();
				debugVals.debugLine.push_back({ toDVec2(p1), toDVec2(p2) });
			}
			debugVals.debugDraw.push_back(p1);
			debugVals.debugDraw.push_back(p2);
		};

		[[maybe_unused]]
		auto polyPoint = [](bc::PloyPoints& p1, bool finale)
		{
			const std::lock_guard<std::mutex> lock(debugVals.drawMutex);

			assert(p1.size() >= 3);
			debugVals.debugPlygon.push_back(std::move(p1));
		};

		if (useAsync)
		{
			//bc::CloudPointsBarcode::drawLine = drawLine;
			//bc::CloudPointsBarcode::drawPlygon = polyPoint;
		}

		// Lua bidning
		//lua.bindBarcode();
		//lua.bindLayers();

		// // Bind vector layer funcs
		// CreateVectorLayerFunc createVec = []()
		// {
		// 	VectorLayer* lay = backend.proj->addLayerData<VectorLayer>();
		// 	layersVals.addLayer<VectorGuiLayer>("New layer", lay);
		// 	return lay;
		// };

		// FindVectorLayerByNameFunc findVec = [](const char* name)
		// {
		// 	return dynamic_cast<VectorLayer*>(backend.proj->layers.at(name));
		// };

		// FindVectorLayerByIdFunc findVecId = [](int id)
		// {
		// 	return dynamic_cast<VectorLayer*>(backend.proj->layers.at(id));
		// };

		// DropLayer drop = [](int id)
		// {
		// 	backend.proj->layers.remove(id);
		// };

		// lua.bindVector(createVec, findVec, findVecId, drop);


		// // Bind raster layer funcs
		// FindRasterLayerByNameFunc rastByName = [](const char* name)
		// {
		// 	return dynamic_cast<RasterLayer*>(backend.proj->layers.at(name));
		// };

		// FindRasterLayerByIdFunc rastById = [](int id)
		// {
		// 	return dynamic_cast<RasterLayer*>(backend.proj->layers.at(id));
		// };

		// lua.bindRaster(rastByName, rastById);
	}

	// Main
	void RenderUI()
	{
		//ImGuiID dockspace_id = 0;
		//ImGuiWindowFlags window_flags = 0;

		//const ImGuiViewport* viewport = ImGui::GetMainViewport();

		//bool opt_fullscreen = true;
		//bool opt_padding = false;



		drawLayout();
		//ImGui::ShowDemoWindow();
	}

	void Close()
	{
		backend.save();
		AlgFactory::deleteRaster();
		AlgFactory::deleteVector();
	}
}
