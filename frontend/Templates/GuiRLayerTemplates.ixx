module;
#include "../DrawCommon.h"
#include <memory>
#include <future>

export module GuiRLayersTemplates;
import FrontendBind;
import IItemModule;
import GuiWidgets;
import RasterLayers;
import GuiLayers;
import GuiOverlap;


GuiBackend backend;
int maxThreadCount, minThreadCount;

export struct GuiFilter
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
		{0, "Без фильтра"},
		{1, "Простой"},
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
		typeCB.drawCombobox("Тип");
		switch (typeCB.currentValue())
		{
		case 0:
			break;
		case 1:
			ImGui::Text("Пороги отсеивания");
			_drawPair("Мин. Начало", "Макс. Начало", valsFilter.start);
			_drawPair("Мин. Длина", "Макс. Длина", valsFilter.len);
			_drawPair("Мин. Размер матрицы в %", "Макс. размер матрицы в %", valsFilter.matrSizeProc, 100);
			_drawPair("Мин. Глубина", "Макс. Глубина", valsFilter.depth, 200);
			ImGui::InputInt("Мин. объем матрицы", &valsFilter.minPixelsSize); // matr size must be more then this

			break;
		//case 2:
		//	ImGui::InputTextMultiline("Lua скрипт", text, 10000, ImVec2(500, 300));
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
		auto* core = GuiLayerData<T>::data;

		if (!core->visible)
			return;

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
		{bc::ComponentType::Component, "Компонента"},
		{bc::ComponentType::Hole, "Дыра"}
	};
	// ---

	// Proc Type
	SelectableKeyValues<bc::ProcType> procCB =
	{
		{bc::ProcType::f0t255, "От 0 до 255"},
		{bc::ProcType::f255t0, "От 255 до 0"},
		{bc::ProcType::Radius, "По расстоянию"},
		{bc::ProcType::invertf0, "Инвертировать"},
		{bc::ProcType::experiment, "Радар"},
		// {bc::ProcType::ValueRadius, "Тру расстояние"}
	};

	SelectableKeyValues<bc::ColorType> colorCB =
	{
		{bc::ColorType::native, "Как в изображении"},
		{bc::ColorType::gray, "Серый"},
		{bc::ColorType::rgb, "Цветной"},
	};

	SelectableKeyValues<bc::AttachMode> attachCB =
	{
		{bc::AttachMode::firstEatSecond, "firstEatSecond"},
		{bc::AttachMode::secondEatFirst, "secondEatFirst"},
		{bc::AttachMode::createNew, "createNew"},
		{bc::AttachMode::dontTouch, "dontTouch"},
		{bc::AttachMode::morePointsEatLow, "morePointsEatLow"},
		{bc::AttachMode::closer, "closer"}
	};

	SelectableKeyValues<int> alg =
	{
		{0, "Растровый"},
		{1, "Растр в точки"}
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
		properties.attachMode = attachCB.currentValue();
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
		if (ImGui::Button("Построить баркод"))
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

		if (ImGui::BeginPopupModal("SelectMax", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
			if (ImGui::BeginTabBar("Настройки", tab_bar_flags))
			{
				if (ImGui::BeginTabItem("Алгоритм"))
				{
					if (alg.currentIndex == 0)
					{
						componentCB.drawCombobox("##Форма");
						procCB.drawCombobox("##Обработка");
						colorCB.drawCombobox("##Цвет");
						attachCB.drawCombobox("##Attach");
					}
					else
					{
						ImGui::Checkbox("Использовать дыры", &properties.alg1UseHoles);
						ImGui::Checkbox("Игнорировать высоту", &properties.alg1IgnoreHeight);
					}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Пороги отсеивания"))
				{
					ImGui::Separator();
					ImGui::Text("Пороги отсеивания");
					filterInfo.draw();
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Оптимизация"))
				{
					ImGui::Text("Лимит кэша");

					ImGui::SameLine();
					ImGui::SetNextItemWidth(150);
					ImGui::InputInt("MB", &cacheMb, 1);

					ImGui::Separator();

					if (imgSubImages.getSize() > 1)
					{
						imgSubImages.drawListBox("Размеры");
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

					SubImgInf& sub = subImgs[imgSubImages.currentIndex];
					const int maxSize = std::max(sub.wid, sub.hei);
					if (maxSize >= 10)
					{
						if (ImGui::BeginChild("Tile size", ImVec2(300, 360)))
						{
							ImGui::Text("Размер тайла");
							tileSizeSlider.draw("##Tile size", newTileSize, 10, maxSize, 10);

							int maxOffset = newTileSize;
							if (newTileSize + maxOffset > maxSize)
								maxOffset = maxSize - newTileSize;

							ImGui::Text("Доп. наложение тайла");
							offsetSlider.draw("##Offset size", newOffsetSize, 0, maxOffset, 1);

							ImGui::Separator();
							tilePrview.draw(newTileSize, newOffsetSize, ImVec2(sub.wid, sub.hei));
						}
						ImGui::EndChild();
					}
					//else
					//{
					//	newTileSize = maxSize;
					//	newOffsetSize = 0;
					//}
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}

			ImGui::Separator();
			if (ImGui::Button("Запустить"))
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
			if (ImGui::Button("Отмена"))
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
		ImGui::SliderFloat("Прозрачность", &tempVal, 0.f, 1.f);
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
		{bc::ProcType::f0t255, "От 0 до 255"},
		{bc::ProcType::f255t0, "От 255 до 0"},
		{bc::ProcType::Radius, "По расстоянию"},
		{bc::ProcType::invertf0, "Инвертировать"},
		{bc::ProcType::experiment, "Радар"},
		// {bc::ProcType::ValueRadius, "Тру расстояние"}
	};

public:
	RasterToolsLayer(T* fromCore = nullptr) : TiledRasterGuiLayer<T>(fromCore)
	{ }

	virtual void drawToolboxInner(ILayerWorker& context)
	{
		TiledRasterGuiLayer<T>::drawToolboxInner(context);

		procCB.drawCombobox("Тип");

		if (ImGui::Button("Функция активации"))
		{
			//auto rets = proj->exeFilter(context.iol, 0);
			auto rets = backend.exeFilter(context.iol, procCB.currentValue(), 0);
			context.setLayers(rets, "filter");
		}


		if (ImGui::Button("Квадратичная"))
		{
			//auto rets = proj->exeFilter(context.iol, 0);
			auto rets = backend.exeFilter(context.iol, procCB.currentValue(), 1);
			context.setLayers(rets, "new bar");
		}

		ImGui::Separator();
		static int startEnergy = 100;
		ImGui::InputInt("Начальная энергия", &startEnergy, 1);
		if (ImGui::Button("Энергия"))
		{
			//auto rets = proj->exeFilter(context.iol, 0);
			auto rets = backend.proj->exeEnergy(context.iol, procCB.currentValue(), startEnergy, true);
			context.setLayers(rets, "new cells");
		}
		if (ImGui::Button("Энергия2"))
		{
			//auto rets = proj->exeFilter(context.iol, 0);
			auto rets = backend.proj->exeEnergy(context.iol, procCB.currentValue(), startEnergy, false);
			context.setLayers(rets, "new cells");
		}
		ImGui::Separator();

		if (ImGui::Button("3d"))
		{
			//auto rets = proj->exeFilter(context.iol, 0);
			backend.exeFilter(context.iol, procCB.currentValue(), 3);
		}

		if (ImGui::Button("GUI"))
		{
			ImGui::OpenPopup("UpdateImage");
		}

		if (ImGui::BeginPopupModal("UpdateImage", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			filtere.draw();

			TiledRasterGuiLayer<T>::componentCB.drawCombobox("##Форма");
			TiledRasterGuiLayer<T>::procCB.drawCombobox("##Обработка");
			TiledRasterGuiLayer<T>::colorCB.drawCombobox("##Цвет");

			if (ImGui::Button("Run"))
			{
				TiledRasterGuiLayer<T>::grabSets();
				auto rets = backend.exeGUI(context.iol, TiledRasterGuiLayer<T>::properties, filtere.getFilter());

				ImGui::CloseCurrentPopup();
				context.setLayers(rets, "Разложить");
			}
			ImGui::EndPopup();
		}
		//ImGui::SameLine();
		//ImGui::Checkbox("Переключить вид", &heimap.enable);
		//if (heimap.enable && !heimap.isInit())
		//{
		//	heimap.init(main);
		//}
		ImGui::Separator();
	}
};
