module;
#include "../GuiCommon.h"
#include <memory>
#include <future>

export module GuiBarHolderLayer;
import IItemModule;
import GuiWidgets;
import RasterLayers;
import GuiOverlap;
import RasterBarHolderRLayer;
import GuiRLayersTemplates;
import GuiClusterization;

//GuiBackend backend;


export class RasterLineGuiLayer : public ITiledRasterGuiLayer<GuiDrawCloudPointClick, RasterLineLayer>
{
	using Base = ITiledRasterGuiLayer<GuiDrawCloudPointClick, RasterLineLayer>;
public:
	BackString debug;
	SimpleLine* selectedLine = nullptr;
	GuiFilter filtere;
	GuiClusterizationWindow clusterVals;

	RasterLineGuiLayer(RasterLineLayer* fromCore = nullptr) : ITiledRasterGuiLayer(fromCore)
	{
	}

	float tempVal = 1.f;
	virtual void drawProperty()
	{
		Base::drawProperty();
		ImGui::Separator();
		ImGui::SliderFloat("������������", &tempVal, 0.f, 1.f);
	}
	virtual void applyPropertyChanges()
	{
		Base::applyPropertyChanges();
		main.opacity = tempVal;
	}

	virtual void toGuiData()
	{
		Base::toGuiData();
		main.setImage(Base::data->mat, false);
		icon.setImage(Base::data->mat, 32, 32, true);
		clusterVals.init(Base::data);
	}

	virtual void drawToolboxInner(ILayerWorker& context)
	{
		Base::drawToolboxInner(context);
		ImGui::Separator();

		if (ImGui::Button("Update"))
		{
			ImGui::OpenPopup("UpdateImage");
		}


		if (ImGui::BeginPopupModal("UpdateImage", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			filtere.draw();

			if (ImGui::Button("Update"))
			{
				selectedLine = nullptr;

				ImGui::CloseCurrentPopup();
				//auto rets = backend.processRaster(context.iol, filtere.getFilter());
				auto rets = Base::data->processCachedBarcode(filtere.getFilter());
				this->toGuiData();
				context.setLayers(rets, "barcode");
				//commonValus.onAir = true;
				//commonValus.future = std::async(&GuiBackend::processRaster, std::ref(backend),
			}
			ImGui::SameLine();

			if (ImGui::Button("Close"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine(0, 30);
			if (ImGui::Button("�������������"))
			{
				clusterVals.show = true;
			}

			clusterVals.draw();



			ImGui::EndPopup();
		}
	}

	virtual void draw(const GuiDisplaySystem& ds)
	{
		if (!visible)
			return;

		Base::draw(ds);

		ImVec2 realSize(data->displayWidth(), data->displayHeight());
		main.drawPoints(ds, getCore()->cs, realSize);
		drawLineInfoWin(ds);
	}

	virtual void onClick(const GuiDisplaySystem& ds, BackPoint pos)
	{
		ImVec2 posInDisplay = ds.projItemGlobToDisplay(data->cs, pos);
		if (ds.inDisplayRange(posInDisplay))
		{
			// globP =
			// 1. sub point -> display (mat variable) via tileProv
			// 2. Cast to a real img via factor and save for draw

			// (pos - origin) / factor
			auto ps = data->cs.proj.getThisProj(ds.core.sysProj, pos, false);
			const BackPixelPoint pix = data->cs.toLocal(pos);
			auto points = click(pix.x / data->subToRealFactor, pix.y / data->subToRealFactor);
			setPoints(ds, points);
		}
	}


	void setPoints(const GuiDisplaySystem& ds, const bc::barvector* points)
	{
		main.setPoints(ds, data->cs, points);
	}


	bc::barvector* click(int x, int y)
	{
		if (data->clickResponser.size() == 0)
			return nullptr;

		// x = main.getRealX(x);
		// y = main.getRealY(y);


		if (x < 0 || x >= main.width)
			return nullptr;

		if (y < 0 || y >= main.height)
			return nullptr;

		//std::cout << x << " : " << y << std::endl;
		int index = data->mat.getLineIndex(x, y);
		SimpleLine* line = data->clickResponser[index].get();
		if (line)
		{
			if (selectedLine == line && line->parent)
				line = line->parent.get();

			selectedLine = line;
			return &(selectedLine->matr);
		}

		return nullptr;
	}


	SimpleLine* moveToParenr()
	{
		return selectedLine = selectedLine->parent.get();
	}

	void drawLineInfoWin(const GuiDisplaySystem& ds)
	{
		SimpleLine* line = selectedLine;
		if (!line)
		{
			return;
		}

		if (!ImGui::Begin("Propertis"))
		{
			ImGui::End();
			return;
		}

		if (line->depth > 0)
		{
			ImGui::BeginDisabled(line->parent == nullptr);
			if (ImGui::Selectable("Parent"))
			{
				line = moveToParenr();
				setPoints(ds, &line->matr);
			}
			ImGui::EndDisabled();

			ImGui::Separator();
			BackString s = line->start.text<BackString, toStdStr>();
			s = "Start: " + s;
			ImGui::Text(s.c_str());
			s = line->end.text<BackString, toStdStr>();
			s = "End: " + s;
			ImGui::Text(s.c_str());

			ImGui::Text("Depth %d", line->depth);
			ImGui::Text("Matr size %d", line->matrSrcSize);
		}
		else
		{
			ImGui::Text("The root has been reached");
		}

		ImGui::End();
	}
private:
};
