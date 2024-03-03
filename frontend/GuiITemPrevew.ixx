module;
#include <algorithm>
#include <memory>
#include "../side/implot/implot.h"
#include <vector>

export module GuiITemPrevew;
import LayersCore;
import RasterLineLayerModule;
import ProjectModule;
import Settings;
import GuiLayers;
import GuiWidgets;
import IItemModule;
import ClusterInterface;
import CachedBarcode;
import CSBind;

import IGuiLayer;


export class TemPreview
{
	std::unique_ptr<IClassItemHolder> collectionToPredict;
	std::vector<float> xs;
	std::vector<float> ys;
public:

	TemPreview()
	{
		auto& factory = getClusterFactory();
		for (int i = 0; i < factory.getCount(); i++)
		{
			auto& name = factory.getNames()[i];
			itemMethods.add(name,  i);
		}
		itemMethods.endAdding();
		auto methodId = itemMethods.currentValue();
	}

	RetLayers makePreviwLayer(RasterLineLayer* inLayer, int methodId)
	{
		LayerProvider& prov = inLayer->prov;
		// Line layer
		auto& metaprov = getSettings().getMeta();
		RasterLineLayer* layer = Project::getProject()->addLayerData<RasterLineLayer>(inLayer->cs.getProjId());
		layer->init(inLayer, metaprov);
		layer->initCSFrom(inLayer->cs);
		layer->tileOffset = 0;


		RetLayers ret;
		ret.push_back(layer);

		if (raw)
			collectionToPredict.reset(new CachedBaritemHolder());
		else
			collectionToPredict = getClusterFactory().CreateHolder(methodId);

		inLayer->collectionToPredict = collectionToPredict.get();
		inLayer->processCachedBarcode(nullptr);
		collectionToPredict->perform();

		int inde = 0;
		int tileSize = std::max(inLayer->displayWidth(), inLayer->displayHeight());
		prov.init(inLayer->displayWidth(), inLayer->displayHeight(), inLayer->displayWidth(), tileSize);
		TileProvider tileProv = prov.tileByOffset(0, 0);

		const size_t count = collectionToPredict->getItemsCount();
		xs.resize(count);
		ys.resize(count);
		const IClassItemHolder* const holder = collectionToPredict.get();
		for (size_t i = 0; i < count; i++)
		{
			const IClassItem* item = holder->getItem(i); // we need to call the const version
			layer->addLine(inde++, item, tileProv);
			xs[i] = item->start().getAvgFloat();
			ys[i] = item->end().getAvgFloat();
		}

		//
		// Classes layers
		//for (auto& i : classLayers)
		//{
		//	ret.push_back(i.second);
		//	i.second->clear();
		//	i.second->color = classCategs.get(i.first)->color;
		//	i.second->initCSFrom(inLayer->cs);
		//}
		// -------------------

		// Cacher
		//ItemHolderCache cacher;
		//cacher.openWrite(layer->getCacheFilePath(prov));
		// ------


		// Setup tileIterators
		//TileImgIterator tileIter(tileSize, tileOffset, curSize.wid, curSize.hei);

		// Threads

		return ret;
	}

	ImPlotRect prevRect;
	bool raw = false;
	bool show = false;
	SelectableKeyValues<int> itemMethods;
	void draw(ILayerWorker& context, RasterLineLayer* layer, GuiDrawCloudPointClick& main)
	{
		if (!show)
			return;

		if (!ImGui::Begin("Tem"))
		{
			ImGui::End();
			return;
		}
		ImGui::SameLine();
		ImGui::Checkbox("Raw", &raw);

		ImGui::BeginDisabled(raw);
		itemMethods.drawCombobox("Метод");
		ImGui::EndDisabled();

		if (ImGui::Button("Превью"))
		{
			context.setLayers(makePreviwLayer(layer, itemMethods.currentValue()), "converted");
		}

		CSBinding bind;
		if (ImPlot::BeginPlot("##Centroid"))
		{
			ImPlot::SetupAxesLimits(0, 256, 0, 256);
	/*		if (ImPlot::IsPlotHovered() && ImGui::IsMouseClicked(0) && ImGui::GetIO().KeyCtrl)
			{
				ImPlotPoint pt = ImPlot::GetPlotMousePos();
				data.push_back(pt);
			}*/
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				ImPlot::CancelPlotSelection();
			}

			ImPlot::PlotScatter("Points", xs.data(), ys.data(), xs.size(), 0, 0, sizeof(float));
			ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
			ImPlot::PopStyleVar();
			if (ImPlot::IsPlotSelected())
			{
				ImPlotRect rect = ImPlot::GetPlotSelection();
				if (prevRect.X.Min != rect.X.Min || prevRect.Y.Min != rect.Y.Min ||
					prevRect.X.Max != rect.X.Max || prevRect.Y.Max != rect.Y.Max)
				{
					main.unsetPoints();
				}

				prevRect = rect;


				for (size_t i = 0; i < ys.size(); i++)
				{
					if (rect.Contains(ImPlotPoint(xs[i], ys[i])))
					{
						//main.setPoints(bind, collectionToPredict.get()->getItem(i)->getMatrix(), false);
						main.setPoints(bind, &layer->holder.holder[i]->matr, false);
						//main.setPoints(bind, &layer->clickResponser[ys[i] * layer->mat.wid() + xs[i]]->matr);
					}
				}
			}
			ImPlot::EndPlot();
		}

		ImGui::End();
	}
};
