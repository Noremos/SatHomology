// module;
#include "GuiVectorLayers.h"


void drawTree(const GuiDisplaySystem& ds, TreeVectorGuiLayer* layer, VecTree& tree, int& counter)
{
	//if (tree.size == -1 && tree.children.size() == 0)
	//	return;

	++counter;


	static int node_clicked = 0;
	ImGuiTreeNodeFlags node_flags = 0;
	if (counter == node_clicked)
	{
		node_flags |= ImGuiTreeNodeFlags_Selected; // ImGuiTreeNodeFlags_Bullet
		layer->spotId = tree.primId;
	}

	int size = tree.size; //layer->getPrim(tree.primId)->points.size();
	if (tree.children.size() == 0)
	{
		node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		ImGui::TreeNodeEx((void*)(intptr_t)counter, node_flags, "%d", size);
		if (ImGui::IsItemClicked())
			node_clicked = counter;

	}
	else
	{
		node_flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (ImGui::TreeNodeEx((void*)(intptr_t)counter, node_flags, "%d", size))
		{
			if (ImGui::IsItemClicked())
				node_clicked = counter;

			for (VecTree& i : tree.children)
			{
				drawTree(ds, layer, i, counter);
			}
			ImGui::TreePop();
		}
		else if (ImGui::IsItemClicked())
			node_clicked = counter;
	}
}
