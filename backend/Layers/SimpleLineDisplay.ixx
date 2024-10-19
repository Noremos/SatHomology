//export module SimpleLine;
////import DrawUtils;
#include "../DrawUtils.h"
////import VectorLayers;
#include "../backend/Layers/VectorLayers.h"
//
//export void MyFunc();

//
//void drawTree(const GuiDisplaySystem& ds, TreeVectorGuiLayer* layer, VecTree& tree, int& counter)
//{
//	//if (tree.size == -1 && tree.children.size() == 0)
//	//	return;
//
//	++counter;
//
//
//	static int node_clicked = 0;
//	ImGuiTreeNodeFlags node_flags = 0;
//	if (counter == node_clicked)
//	{
//		node_flags |= ImGuiTreeNodeFlags_Selected; // ImGuiTreeNodeFlags_Bullet
//		layer->spotId = tree.primId;
//	}
//
//	int size = tree.size; //layer->getPrim(tree.primId)->points.size();
//	if (tree.children.size() == 0)
//	{
//		node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
//		ImGui::TreeNodeEx((void*)(intptr_t)counter, node_flags, "%d", size);
//		if (ImGui::IsItemClicked())
//			node_clicked = counter;
//
//	}
//	else
//	{
//		node_flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
//		if (ImGui::TreeNodeEx((void*)(intptr_t)counter, node_flags, "%d", size))
//		{
//			if (ImGui::IsItemClicked())
//				node_clicked = counter;
//
//			for (VecTree& i : tree.children)
//			{
//				drawTree(ds, layer, i, counter);
//			}
//			ImGui::TreePop();
//		}
//		else if (ImGui::IsItemClicked())
//			node_clicked = counter;
//	}
//}
//
//
//
//void drawLineInfoWin(const GuiDisplaySystem& ds)
//{
//	SimpleLine* line = selectedLine;
//	if (!line)
//	{
//		return;
//	}
//
//	if (!ImGui::Begin("Propertis"))
//	{
//		ImGui::End();
//		return;
//	}
//
//	if (line->depth > 0)
//	{
//		ImGui::BeginDisabled(line->parent == nullptr);
//		if (ImGui::Selectable("Parent"))
//		{
//			line = moveToParenr();
//			setPoints(ds, &line->matr);
//		}
//		ImGui::EndDisabled();
//
//		ImGui::Separator();
//		BackString s = line->start.text<BackString, toStdStr>();
//		s = "Start: " + s;
//		ImGui::Text(s.c_str());
//		s = line->end.text<BackString, toStdStr>();
//		s = "End: " + s;
//		ImGui::Text(s.c_str());
//
//		ImGui::Text("Depth %d", line->depth);
//		ImGui::Text("Matr size %d", line->matrSrcSize);
//	}
//	else
//	{
//		ImGui::Text("The root has been reached");
//	}
//
//	ImGui::End();
//}