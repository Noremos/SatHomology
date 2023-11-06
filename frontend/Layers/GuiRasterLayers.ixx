module;
#include "../GuiCommon.h"
#include <memory>
#include <future>

export module GuiRasterLayers;
import FrontendBind;
import IItemModule;
import GuiWidgets;
import RasterLayers;
import GuiLayers;
import GuiOverlap;


GuiBackend backend;
int maxThreadCount, minThreadCount;

struct GuiFilter
{
	RangeItemFilter valsFilter;
	//ScriptFilterInfo scriptFilter;
	char text[10000];

	GuiFilter()
	{
		//const BackString re = ScriptFilterInfo::getScriptTemplate();
		//memcpy(text, re.c_str(), re.length());
	}

	SelectableKeyValues<int> typeCB =
	{
		{0, "��� �������"},
		{1, "�������"},
		{2, "Lua script"}
	};

	void _drawPair(const char* name1, const char* name2, RangeItemFilter::FRange& rng, int max = 256)
	{
		ImGui::SliderInt(name1, &rng.first, 0, max, "%d");
		ImGui::SliderInt(name2, &rng.second, 0, max, "%d");
	}

	IItemFilter* getFilter()
	{
		int id = typeCB.currentValue();
		switch (id)
		{
		case 0:
			return nullptr;
		case 1:
			return &valsFilter;
			//case 2:
			//{
			//	scriptFilter.setScript(text);
			//	return &scriptFilter;
			//}
		}
		return nullptr;
	}

	void draw()
	{
		typeCB.drawCombobox("���");
		switch (typeCB.currentValue())
		{
		case 0:
			break;
		case 1:
			ImGui::Text("������ ����������");
			_drawPair("���. ������", "����. ������", valsFilter.start);
			_drawPair("���. �����", "����. �����", valsFilter.len);
			_drawPair("���. ������ ������� � %", "����. ������ ������� � %", valsFilter.matrSizeProc, 100);
			_drawPair("���. �������", "����. �������", valsFilter.depth, 200);
			ImGui::InputInt("���. ����� �������", &valsFilter.minPixelsSize); // matr size must be more then this

			break;
			//case 2:
			//	ImGui::InputTextMultiline("Lua ������", text, 10000, ImVec2(500, 300));
			//	break;
		default:
			break;
		}
	}
};


export template<class IM, class T>
class ITiledRasterGuiLayer : public GuiLayerData<T>
{
public:
	IM main;
	GuiTilePreview tilePrview;
	int newTileSize;
	int newOffsetSize;

	ITiledRasterGuiLayer(T* fromCore) : GuiLayerData<T>(fromCore)
	{ }

	virtual ~ITiledRasterGuiLayer()
	{ }

	virtual void toGuiData()
	{
		GuiLayerData<T>::toGuiData();

		newTileSize = getTileSize();
		newOffsetSize = getOffsetSize();
	}

	virtual void draw(const GuiDisplaySystem& ds)
	{
		if (!IGuiLayer::visible)
			return;

		auto* core = GuiLayerData<T>::data;
		BackPixelPoint realSize(core->realWidth(), core->realHeight());

		auto wpos = ds.getWinPos();
		auto start = ds.projItemLocalToDisplay(core->cs, { 0,0 });
		auto end = ds.projItemLocalToDisplay(core->cs, realSize);

		main.drawImage(GuiLayerData<T>::getName(), wpos, ds.getDrawSize(), start, end);
	}

	inline int getTileSize() const
	{
		return GuiLayerData<T>::getProvider().tileSize;
	}
	inline int getOffsetSize() const
	{
		return GuiLayerData<T>::data->tileOffset;
	}

	inline ImVec2 getImageSize() const
	{
		return ImVec2(GuiLayerData<T>::data->realWidth(), GuiLayerData<T>::data->realHeight());
	}

	SelectableKeyValues<int> imgSubImages;

	// Component
	SelectableKeyValues<bc::ComponentType> componentCB =
	{
		{bc::ComponentType::Component, "����������"},
		{bc::ComponentType::Hole, "����"}
	};
	// ---

	// Proc Type
	SelectableKeyValues<bc::ProcType> procCB =
	{
		{bc::ProcType::f0t255, "�� 0 �� 255"},
		{bc::ProcType::f255t0, "�� 255 �� 0"},
		{bc::ProcType::Radius, "�� ����������"},
		{bc::ProcType::invertf0, "�������������"},
		{bc::ProcType::experiment, "�����"},
		// {bc::ProcType::ValueRadius, "��� ����������"}
	};

	SelectableKeyValues<bc::ColorType> colorCB =
	{
		{bc::ColorType::native, "��� � �����������"},
		{bc::ColorType::gray, "�����"},
		{bc::ColorType::rgb, "�������"},
	};

	SelectableKeyValues<int> alg =
	{
		{0, "���������"},
		{1, "����� � �����"}
	};
	BarcodeProperies properties;
	GuiFilter filterInfo;
	int cacheMb = 1024;
	int threads = 1;

	void grabSets()
	{
		properties.barstruct.proctype = procCB.currentValue();
		properties.barstruct.coltype = colorCB.currentValue();
		properties.barstruct.comtype = componentCB.currentValue();
		properties.alg = alg.currentIndex;
	}

	void createBarcode(ILayerWorker& context)
	{
		grabSets();
		RetLayers layerData = backend.createBarcode(context.iol, properties, filterInfo.getFilter());
		//RetLayers layerData = proj->createCacheBarcode(context.iol, properties, filterInfo.getFilter());
		context.setLayers(layerData, "barcode");
	}
	StepIntSlider tileSizeSlider, offsetSlider;
	std::vector<SubImgInf> subImgs;

	virtual void drawToolboxInner(ILayerWorker& context)
	{
		if (ImGui::Button("��������� ������"))
		{
			subImgs = GuiLayerData<T>::data->getSubImageInfos();
			if (subImgs.size() != 0)
			{
				imgSubImages.clear();
				for (size_t i = 0; i < subImgs.size(); i++)
				{
					SubImgInf& sub = subImgs[i];
					BackString s = intToStr(sub.wid) + "x" + intToStr(sub.hei);
					imgSubImages.add(s, i);
				}
				imgSubImages.endAdding();
				imgSubImages.currentIndex = 0;
			}
			else
			{
				subImgs.push_back(BackSize(GuiLayerData<T>::data->realWidth(), GuiLayerData<T>::data->realHeight()));
			}
			ImGui::OpenPopup("SelectMax");
		}

		if (ImGui::BeginPopupModal("SelectMax", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
			if (ImGui::BeginTabBar("���������", tab_bar_flags))
			{
				if (ImGui::BeginTabItem("��������"))
				{
					if (alg.currentIndex == 0)
					{
						componentCB.drawCombobox("##�����");
						procCB.drawCombobox("##���������");
						colorCB.drawCombobox("##����");

					}
					else
					{
						ImGui::Checkbox("������������ ����", &properties.alg1UseHoles);
						ImGui::Checkbox("������������ ������", &properties.alg1IgnoreHeight);
					}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("������ ����������"))
				{
					ImGui::Separator();
					ImGui::Text("������ ����������");
					filterInfo.draw();
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("�����������"))
				{
					ImGui::Text("����� ����");

					ImGui::SameLine();
					ImGui::SetNextItemWidth(150);
					ImGui::InputInt("MB", &cacheMb, 1);

					ImGui::Separator();

					if (imgSubImages.getSize() > 1)
					{
						imgSubImages.drawListBox("�������");
						if (imgSubImages.hasChanged())
						{
							GuiLayerData<T>::data->setSubImage(imgSubImages.currentIndex);
							SubImgInf& sub = subImgs[imgSubImages.currentIndex];
							int maxSize = std::max(sub.wid, sub.hei);
							if (newTileSize > maxSize)
							{
								newTileSize = maxSize;
							}
						}
						ImGui::SameLine();
					}

					if (ImGui::BeginChild("Tile size", ImVec2(300, 360)))
					{
						SubImgInf& sub = subImgs[imgSubImages.currentIndex];
						int maxSize = std::max(sub.wid, sub.hei);
						ImGui::Text("������ �����");
						tileSizeSlider.draw("##Tile size", newTileSize, 10, maxSize, 10);

						int maxOffset = newTileSize;
						if (newTileSize + maxOffset > maxSize)
							maxOffset = maxSize - newTileSize;

						ImGui::Text("���. ��������� �����");
						offsetSlider.draw("##Offset size", newOffsetSize, 0, maxOffset, 1);

						ImGui::Separator();
						tilePrview.draw(newTileSize, newOffsetSize, ImVec2(sub.wid, sub.hei));
					}
					ImGui::EndChild();
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}

			ImGui::Separator();
			if (ImGui::Button("���������"))
			{
				GuiLayerData<T>::data->prov.tileSize = newTileSize;
				GuiLayerData<T>::data->tileOffset = newOffsetSize;

				GuiLayerData<T>::data->setCache(cacheMb * 1024 * 1024);
				GuiLayerData<T>::data->setSubImage(imgSubImages.currentIndex);

				ImGui::CloseCurrentPopup();
				createBarcode(context);
				GuiLayerData<T>::data->setSubImage(0);
			}

			ImGui::SameLine();
			if (ImGui::Button("������"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
};

export template<class T>
class TiledRasterGuiLayer : public ITiledRasterGuiLayer<GuiDrawImage, T>
{
	using Base = ITiledRasterGuiLayer<GuiDrawImage, T>;
public:
	TiledRasterGuiLayer(T* fromCore) : ITiledRasterGuiLayer<GuiDrawImage, T>(fromCore)
	{ }

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
		Base::main.opacity = tempVal;
	}

	virtual ~TiledRasterGuiLayer()
	{ }
};


export template<class T>
class RasterToolsLayer : public TiledRasterGuiLayer<T>
{
	GuiFilter filtere;

	// Proc Type
	SelectableKeyValues<bc::ProcType> procCB =
	{
		{bc::ProcType::f0t255, "�� 0 �� 255"},
		{bc::ProcType::f255t0, "�� 255 �� 0"},
		{bc::ProcType::Radius, "�� ����������"},
		{bc::ProcType::invertf0, "�������������"},
		{bc::ProcType::experiment, "�����"},
		// {bc::ProcType::ValueRadius, "��� ����������"}
	};

public:
	RasterToolsLayer(T* fromCore = nullptr) : TiledRasterGuiLayer<T>(fromCore)
	{ }

	virtual void drawToolboxInner(ILayerWorker& context)
	{
		TiledRasterGuiLayer<T>::drawToolboxInner(context);

		procCB.drawCombobox("���");

		if (ImGui::Button("������� ���������"))
		{
			//auto rets = proj->exeFilter(context.iol, 0);
			auto rets = backend.exeFilter(context.iol, procCB.currentValue(), 0);
			context.setLayers(rets, "filter");
		}


		if (ImGui::Button("������������"))
		{
			//auto rets = proj->exeFilter(context.iol, 0);
			auto rets = backend.exeFilter(context.iol, procCB.currentValue(), 1);
			context.setLayers(rets, "new bar");
		}

		static int startEnergy = 100;
		ImGui::InputInt("��������� �������", &startEnergy, 1);
		if (ImGui::Button("���������"))
		{
			//auto rets = proj->exeFilter(context.iol, 0);
			auto rets = backend.proj->exeEnergy(context.iol, procCB.currentValue(), startEnergy);
			context.setLayers(rets, "new cells");
		}

		if (ImGui::Button("3d"))
		{
			//auto rets = proj->exeFilter(context.iol, 0);
			backend.exeFilter(context.iol, procCB.currentValue(), 3);
		}

		if (ImGui::Button("GUI"))
		{
			ImGui::OpenPopup("UpdateImage");
		}

		if (ImGui::BeginPopupModal("UpdateImage", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			filtere.draw();

			TiledRasterGuiLayer<T>::componentCB.drawCombobox("##�����");
			TiledRasterGuiLayer<T>::procCB.drawCombobox("##���������");
			TiledRasterGuiLayer<T>::colorCB.drawCombobox("##����");

			if (ImGui::Button("Run"))
			{
				TiledRasterGuiLayer<T>::grabSets();
				auto rets = backend.exeGUI(context.iol, TiledRasterGuiLayer<T>::properties, filtere.getFilter());

				ImGui::CloseCurrentPopup();
				context.setLayers(rets, "���������");
			}
			ImGui::EndPopup();
		}
		//ImGui::SameLine();
		//ImGui::Checkbox("����������� ���", &heimap.enable);
		//if (heimap.enable && !heimap.isInit())
		//{
		//	heimap.init(main);
		//}
		ImGui::Separator();
	}
};



export class RasterGuiLayer : public RasterToolsLayer<RasterLayer>
{
public:

	RasterGuiLayer(RasterLayer* fromCore = nullptr) : RasterToolsLayer<RasterLayer>(fromCore)
	{ }

	virtual void toGuiData()
	{
		RasterToolsLayer::toGuiData();

		tempVal = 1.f;
		main.setImage(data->mat, false);
		icon.setImage(data->mat, 32, 32, true);
	}

	virtual void drawToolboxInner(ILayerWorker& context)
	{
		RasterToolsLayer<RasterLayer>::drawToolboxInner(context);

		if (ImGui::Button("���������"))
		{
			BackPathStr path = getSavePath({ "png", "*.png",
								"jpg", "*.jpg" });
			imwrite(path, data->mat);
		}
		ImGui::Separator();

	}
};


export class RasterLineGuiLayer : public ITiledRasterGuiLayer<GuiDrawCloudPointClick, RasterLineLayer>
{
	using Base = ITiledRasterGuiLayer<GuiDrawCloudPointClick, RasterLineLayer>;
public:
	BackString debug;
	SimpleLine* selectedLine = nullptr;
	GuiFilter filtere;

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
				auto rets = backend.processRaster(context.iol, filtere.getFilter());
				//auto rets = proj->processCachedBarcode(context.iol, filtere.getFilter());
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


export class RasterFromDiskGuiLayer : public RasterToolsLayer<RasterFromDiskLayer>
{
public:
	HeimapOverlap heimap;
	TilemapOverlap tilemap;

	bool drawHeimap;

	RasterFromDiskGuiLayer(RasterFromDiskLayer* fromCore = nullptr) : RasterToolsLayer<RasterFromDiskLayer>(fromCore)
	{
		drawHeimap = false;
		heimap.enable = false;
	}

	virtual void toGuiData()
	{
		TiledRasterGuiLayer::toGuiData();

		auto i = *data->getCachedImage();
		main.setImage(i, false);
		icon.setImage(i, 32, 32, true);
	}

	bool isDrawing = false;
	ImVec2 rectMin, rectMax;
	virtual void onClick(const GuiDisplaySystem& ds, BackPoint pos)
	{
		ImVec2 posInDisplay = ds.projItemGlobToDisplay(data->cs, pos);
		if (ds.inDisplayRange(posInDisplay))
		{
			isDrawing = true;
			rectMin = ImGui::GetMousePos();
			rectMax = rectMin;
		}
	}

	GuiResizableContainer selectedRect;
	GuiDrawImage zoomImg;
	virtual void draw(const GuiDisplaySystem& ds)
	{
		if (!visible)
			return;

		TiledRasterGuiLayer::draw(ds);

		ImVec2 winpos = ImGui::GetCurrentWindow()->Pos;

		if (selectedRect.Begin("Zoom"))
		{
			ImGuiWindow* win = ImGui::GetCurrentWindow();

			ImVec2 diplsyPos = selectedRect.offset;
			ImVec2 diplsySize = ImVec2(zoomImg.width, zoomImg.height) * selectedRect.getZoom();

			zoomImg.drawImage("Part", ImVec2(0, 30), win->Size, ImVec2(0, 0), diplsySize);
			selectedRect.end(win->Pos, win->Size);
		}


		// Draw the rectangle if we're currently drawing it
		if (!isDrawing)
			return;

		auto window_flags = ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollWithMouse;
		ImGui::SetCursorPos(ds.getWinPos());
		if (ImGui::BeginChild("Draw rect", ds.getDrawSize(), false, window_flags))
		{
			// Get the draw list for the current window
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			// Check if the left mouse button is released
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				// Stop drawing the rectangle
				isDrawing = false;
				auto absP = ds.core.toSysGlob(toBP(rectMin - winpos));
				auto ps = data->cs.proj.getThisProj(ds.core.sysProj, absP, false);
				const BackPixelPoint pixSt = data->cs.toLocal(ps);

				absP = ds.core.toSysGlob(toBP(rectMax - winpos));
				ps = data->cs.proj.getThisProj(ds.core.sysProj, absP, false);
				const BackPixelPoint pixEnd = data->cs.toLocal(ps);

				data->setSubImage(0);
				zoomImg.setImage(data->getRect(pixSt.x, pixSt.y, pixEnd.x - pixSt.x, pixEnd.y - pixSt.y), false);
				selectedRect.open = true;
			}
			else
			{
				rectMax = ImGui::GetMousePos();
				ImU32 color = IM_COL32(0, 0, 255, 120);
				float thickness = 2.0f;
				drawList->AddRectFilled(rectMin, rectMax, color, 0, ImDrawCornerFlags_All);
			}
		}
		ImGui::EndChild();
	}


	virtual void drawOverlap(const GuiDisplaySystem& ds)
	{
		if (drawHeimap)
		{
			//heimap.draw(ds);
		}
		else
		{
			// tilemap.init(&main, &getProvider());
			// tilemap.draw(main.displaysBegin, main.displaySize);
		}
	}

	std::vector<SubImgInf> getSubImageInfos()
	{
		return data->getSubImageInfos();
	}
};
