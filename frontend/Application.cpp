// https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
#include "Application.h"
#include "GuiCommon.h"

#include <cmath>
#include <initializer_list>
#include <memory>
#include <future>


//#include "sol3/sol.hpp"
//
//#include <GLFW/glfw3.h>
//#include "../libs/glew/include/GL/glew.h"

import ForntnedModule;
import Platform;
import LayersGui;
import BarcodeModule;
import GuiOverlap;
import GuiClassifierModule;
import GuiWidgets;
import IOCore;
import VectorLayers;

//import Lua;

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

	//template<class T>
	//struct ValBinder
	//{
	//	T* from;
	//	T* to;
	//};

	GuiBackend backend;
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


	void clearBeforeCreate();


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

		bool openPop = false;
		bool openCoordSystemPopup = false;
		std::vector<BackString> names;
		int selectedName = 0;
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

	void unsetPoints()
	{
		//centerVals.clickHandler.points = nullptr;
		//centerVals.processImage.clicked = false;
	}

	void clearBeforeCreate()
	{
		unsetPoints();
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
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;
		float heighto = ImGui::GetFrameHeight();
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
					auto* core = backend.loadImageOrProject(path);
					if (backend.isImageLoaded())
					{

						//centerVals.tilemap.init(tileSize = backend.getTileSize();
						//classerVals.loadClassImages();
					}

					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0)))
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			static RasterFromDiskLayer* layer;

			ImGui::SameLine();
			if (ImGui::Button("Load prj"))
			{
				BackPathStr path = openImageOrProject();
				if (!path.empty())
				{
					layer = backend.loadImageOrProject(path);
					if (backend.isImageLoaded())
					{
						if (layer)
						{
							if (layer->hasCS())
							{
								tbVals.enableProcessBtn = true;
								auto* guiLayer = layersVals.addLayer<RasterFromDiskGuiLayer>("Loaded", layer);
								guiLayer->lockAtThis();
								layer = nullptr;
							}
							else
							{
								BackString dps = DEFAULT_PROJECTION_STR;
								tbVals.names = BackProj::getWtkNames();
								for (int i = 0; i < tbVals.names.size(); i++)
								{
									if (tbVals.names[i] == dps)
									{
										tbVals.selectedName = i;
										break;
									}
								}

								// *layer->cs.getScaleX() = 10.0;
								// *layer->cs.getScaleY() = 10.0;
								ImGui::OpenPopup("SetupCS");
							}
						}
						else
							layersVals.loadLayers();
						//layer->data = backend.getMain();
						// centerVals.heimap.init(layer->getData()->mat);
						// centerVals.tilemap.init(&layer->main, backend.getTileSize());
						classerVals.init();
						unsetPoints();
					}
				}
			}

			if (ImGui::BeginPopupModal("SetupCS", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				if (ImGui::BeginCombo("Select CS", tbVals.names[tbVals.selectedName].c_str()))
				{
					int k = 0;
					for (auto& n : tbVals.names)
					{
						bool seled = ImGui::Selectable(n.c_str());
						if (seled)
						{
							tbVals.selectedName = k;
						}
						++k;
					}

					ImGui::EndCombo();
				}

				ImGui::InputDouble("origin x", &layer->cs.globOrigin.x);
				ImGui::InputDouble("origin y", &layer->cs.globOrigin.y);

				ImGui::InputDouble("Scale x", layer->cs.getScaleX());
				ImGui::InputDouble("Scale y", layer->cs.getScaleY());

				ImGui::Separator();
				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					layer->cs.init(tbVals.names[tbVals.selectedName].c_str());
					auto* guiLayer = layersVals.addLayer<RasterFromDiskGuiLayer>("Loaded", layer);
					guiLayer->lockAtThis();
					layer = nullptr;

					tbVals.enableProcessBtn = true;
					tbVals.names.clear();
					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0)))
				{
					backend.removeLayer(layer->id);
					layer = nullptr;

					tbVals.names.clear();
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			// Always center this window when appearing
			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			ImGui::SameLine();
			ImGui::BeginDisabled(!tbVals.enableProcessBtn);

			ImGui::SameLine();
			if (ImGui::Button("Свойства слоя"))
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

					IRasterLayer* core = layersVals.getCurrentRasterCore();

					// if (core)
						// centerVals.tilemap.setTilesize(core.prov.tileSize);
				}

				ImGui::SetItemDefaultFocus();
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0)))
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			ImGui::EndDisabled();

			// ---------------------------------
			ImGui::SameLine();
			if (ImGui::Button("Восстановить"))
			{
				backend.restoreSource();
			}

			ImGui::SameLine();
			if (ImGui::Button("Save"))
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

	const float toolbarSize = 50;
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
		// drawSize.x -= 200;

		//ImGui::SetNextWindowViewport(viewport->ID);

		auto window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking;
			window_flags |= ImGuiWindowFlags_HorizontalScrollbar;

		GuiDisplaySystem guiDisplay(backend.getDS());

		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(drawSize);

		guiDisplay.drawPos = {0,0};
		guiDisplay.drawSize = BackPoint(drawSize.x - 10, drawSize.y - 20);
		if (centerVals.resizble.Begin("ImagePreview"))
		{
			layersVals.draw(guiDisplay);
			if (centerVals.resizble.clicked)
			{
				layersVals.onClick(guiDisplay, centerVals.resizble.clickedPos);
				RasterLineGuiLayer* lay = layersVals.getCastCurrentLayer<RasterLineGuiLayer>();
				if (lay && lay->selectedLine)
				{
					classerVals.selceted = { lay->selectedLine->id, lay->selectedLine->barlineIndex};
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
		if (ImGui::BeginViewportSideBar("##BottomBar", NULL, ImGuiDir_Down, 100, window_flags))
		{
			// GBL
			ImGui::BeginDisabled(commonValus.onAir || !tbVals.enableProcessBtn);

			//ImGui::Text("Happy secondary menu bar");

			ImGui::SameLine();
			if (ImGui::Button("Add class"))
			{
				backend.addClassType("");
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
				// backend.undoAddClass();
			}

			ImGui::SameLine();
			if (ImGui::Button("Выгрузить")) {
				// Open a file dialog to select a folder
				backend.exportResult(getSavePath({ "*.png" }));
			}

			ImGui::SameLine();

			if (ImGui::Checkbox("Enable pics", &bottomVals.drawPics))
			{
				backend.showResultPics(bottomVals.drawPics);
			}

			ImGui::SameLine(0, 30);
			if (ImGui::Button("Классификатор"))
			{
				classerVals.show = true;
			}

			ImGui::SameLine(0, 30);
			ImGui::Text(bottomVals.debug.c_str());

			const auto& curpos = centerVals.resizble.currentPos;
			ImGui::Text("%f:%f", curpos.x, curpos.y);

			// GBL
			ImGui::EndDisabled();
			ImGui::End();
		}
	}


	void debugWindow()
	{
		return;
		if (!ImGui::Begin("debug"))
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

		ImGui::End();
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

	void ToolSetDraw()
	{
		if (!ImGui::Begin("Tools"))
		{
			ImGui::End();
			return;
		}

		if (ImGui::Button("Add vector Layer"))
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
		ToolSetDraw();
		layersVals.drawToolbox();
		ImGui::EndDisabled();

		classerVals.drawClassifierWindow();
		//drawClassifierMenu();

		// Subs
		layersVals.drawLayersWindow();

		commonValus.onAirC();

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


	void MyApp::Init(const char* root)
	{
		Variables::setRoot(root);
		backend.getDS().sysProj.init(DEFAULT_PROJECTION);

		LayerFactory::RegisterFactory<RasterGuiLayer, RasterLayer>(RASTER_LAYER_FID);
		LayerFactory::RegisterFactory<RasterLineGuiLayer, RasterLineLayer>(RASTER_LINE_LAYER_FID);
		LayerFactory::RegisterFactory<RasterFromDiskGuiLayer, RasterFromDiskLayer>(RASTER_DISK_LAYER_FID);
		LayerFactory::RegisterFactory<VectorGuiLayer, VectorLayer>(VECTOR_LAYER_FID);
		//LayerFactory::RegisterFactory<VectorGuiLayer, VetorLayer>(VECTOR_LAYER_FID);

		classerVals.ioLayer = layersVals.getIoLayer();
		auto drawLine = [](const bc::point& p1, const bc::point& p2, bool finale)
		{
			const std::lock_guard<std::mutex> lock(debugVals.drawMutex);

			if (finale)
			{
				debugVals.debugLine.clear();
				debugVals.debugLine.push_back({ ImVec2(p1.x, p1.y), ImVec2(p2.x, p2.y) });
			}
			debugVals.debugDraw.push_back(p1);
			debugVals.debugDraw.push_back(p2);
		};

		auto polyPoint = [](bc::PloyPoints& p1, bool finale)
		{
			const std::lock_guard<std::mutex> lock(debugVals.drawMutex);

			assert(p1.size() >= 3);
			debugVals.debugPlygon.push_back(std::move(p1));
		};

		if (useAsync)
		{
			bc::CloudPointsBarcode::drawLine = drawLine;
			bc::CloudPointsBarcode::drawPlygon = polyPoint;
		}

		//Layer a;
		//a.name = "main";
		//a.icon.setSource("D:\\Learning\\BAR\\sybery\\2.png");
		//layersVals.layers.push_back(a);

		//Layer b;
		//b.name = "sub";
		//b.icon.setSource("D:\\Learning\\BAR\\sybery\\2.png");
		//layersVals.layers.push_back(b);

		//Layer c;
		//c.name = "filter";
		//c.icon.setSource("D:\\Learning\\BAR\\sybery\\2.png");
		//layersVals.layers.push_back(c);
	}

	// Main
	void MyApp::RenderUI()
	{
		//ImGuiID dockspace_id = 0;
		//ImGuiWindowFlags window_flags = 0;

		//const ImGuiViewport* viewport = ImGui::GetMainViewport();

		//bool opt_fullscreen = true;
		//bool opt_padding = false;

		ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
		ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);


		drawLayout();

		ImGui::ShowDemoWindow();
	}
}
