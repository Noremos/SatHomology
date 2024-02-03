module;
#include "../DrawCommon.h"

export module GuiVectorLayers;
import VectorLayers;
import GuiWidgets;
import GuiLayers;


export class VectorGuiLayer : public VectorBaseLayer<VectorLayer>
{
public:
	VectorGuiLayer(VectorLayer* fromCore) : VectorBaseLayer<VectorLayer>(fromCore)
	{ }

	static void getRect(const std::vector<BackPoint>& points, ImVec2& itemSt, ImVec2& end)
	{
		itemSt = end = toIV(points[0]);
		for (const BackPoint& p : points)
		{
			if (p.x < itemSt.x)
			{
				itemSt.x = p.x;
			}

			if (p.y < itemSt.y)
			{
				itemSt.y = p.y;
			}

			if (p.x > end.x)
			{
				end.x = p.x;
			}

			if (p.y < end.y)
			{
				end.y = p.y;
			}
		}
	}

};

class TreeVectorGuiLayer;
void drawTree(const GuiDisplaySystem& ds, TreeVectorGuiLayer* layer, VecTree& tree, int& counter);


export class TreeVectorGuiLayer : public VectorBaseLayer<TreeVectorLayer>
{
public:
	TreeVectorGuiLayer(TreeVectorLayer* fromCore) : VectorBaseLayer<TreeVectorLayer>(fromCore)
	{ }

	virtual void draw(const GuiDisplaySystem& ds)
	{
		if (!getCore()->visible)
			return;

		VectorBaseLayer<TreeVectorLayer>::draw(ds);

		int counter = 0;
		if (ImGui::Begin("Tree"))
		{
			drawTree(ds, this, data->tree, counter);
		}
		ImGui::End();
	}

	DrawPrimitive* getPrim(int id)
	{
		return data->primitives[id];
	}
};



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
