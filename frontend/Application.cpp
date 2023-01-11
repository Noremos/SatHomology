// https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
#include "Application.h"
#include "imgui.h"
#include <imgui_internal.h>
#include <cmath>

namespace MyApp
{
    unsigned int tex_id = 0; // Replace this with your own texture id 

    // How to do a toolbar in Dear ImGui.

    const float toolbarSize = 50;
    int menuBarHeight;

    // Get the size of the image 
    int width = 512; // Replace this with your own width value 
    int height = 512; // Replace this with your own height value


    void ResizeImage(int& width, int& height, int max_width, int max_height)
    {
        // Calculate the aspect ratio of the image
        float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);

        // Calculate the maximum width and height that maintain the aspect ratio
        int max_aspect_width = std::round(max_height * aspect_ratio);
        int max_aspect_height = std::round(max_width / aspect_ratio);

        // Use the maximum width or height that maintains the aspect ratio, whichever is smaller
        width = std::min(max_width, max_aspect_width);
        height = std::min(max_height, max_aspect_height);
    }

    void drawMainImage()
    {
        ImGuiWindow* win =ImGui::FindWindowByName("Photo1");
        int newWid = width;
        int newHei = height;

        ResizeImage(newWid, newHei, win->Size.x, win->Size.y - 20);

        ImGui::SetCursorPosX((win->Size.x - newWid) / 2);
        ImGui::SetCursorPosY((win->Size.y - newHei) / 2);
        ImGui::Image((void*)(intptr_t)tex_id, ImVec2(newWid, newHei));// ImVec2(win., height));
    }

    void drawProcessImage()
    {
        ImGuiWindow* win = ImGui::FindWindowByName("Photo2");
        int newWid = width;
        int newHei = height;

        ResizeImage(newWid, newHei, win->Size.x, win->Size.y - 20);

        ImGui::SetCursorPosX((win->Size.x - newWid) / 2);
        ImGui::SetCursorPosY((win->Size.y - newHei) / 2);
        ImGui::Image((void*)(intptr_t)tex_id, ImVec2(newWid, newHei));// ImVec2(win., height));
    }

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

    void drawLayout()
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
        float heighto = ImGui::GetFrameHeight();
        if (ImGui::BeginViewportSideBar("##TopMenu", NULL, ImGuiDir_Up, 50, window_flags))
        {
            //ImGui::Text("Happy secondary menu bar");

            ImGui::SameLine();

            if (ImGui::Button("Load"))
            {
                //auto path = openImageOrProject(); // "D:\\Learning\\BAR\\sybery\\1.png"
                //LoadTextureFromFile(path.c_str(), &tex_id, &width, &height);
                // https://github.com/mlabbe/nativefiledialog
                // https://github.com/aiekick/ImGuiFileDialog
                // https://github.com/samhocevar/portable-file-dialogs
            }
            //    // Image preview window 

            ImGui::SameLine();
            if (ImGui::Button("Button 2")) {
                // Do something when Button 2 is clicked
            }
            ImGui::SameLine();
            if (ImGui::Button("Button 3")) {
                // Do something when Button 3 is clicked
            }
            ImGui::End();
        }


        if (ImGui::BeginViewportSideBar("##BottomBar", NULL, ImGuiDir_Down, 100, window_flags))
        {
            //ImGui::Text("Happy secondary menu bar");
            ImGui::SameLine();
            if (ImGui::Button("Update")) {
                // Do something when Button 1 is clicked
            }
            ImGui::End();
        }


        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        viewport = ImGui::GetMainViewport();
        ImVec2 pos = viewport->WorkPos;
        ImVec2 size = viewport->WorkSize;
        size.x /= 2;

        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
        ImGui::SetNextWindowViewport(viewport->ID);

        window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

        if (ImGui::Begin("Photo1", nullptr, window_flags))
        {
            drawMainImage();
            ImGui::End();
        }

        pos.x += size.x;

        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
        ImGui::SetNextWindowViewport(viewport->ID);

        if (ImGui::Begin("Photo2", nullptr, window_flags))
        {
            drawProcessImage();
            ImGui::End();
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


        //ImGui::ShowDemoWindow();
    }
}
