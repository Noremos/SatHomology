#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include <filesystem>
namespace fs = std::filesystem;

import RasterLayers;
import IBlock;
import LayersCore;
import ProjectModule;
import AlgUtils;
import BackBind;
import MatrModule;
import ConvertItem;
import CachedBarcode;
import MLSettings;
import RefSettings;

class BarSettings
{
public:
	bc::ProcType type;
	void adjustSettings(RefSettings& settings)
	{
		SettingValue comp("type", type,
		{
			{"От 0 до 255", bc::ProcType::f0t255},
			{"От 255 до 0", bc::ProcType::f255t0},
			{"По расстоянию", bc::ProcType::Radius},
		});
		settings.add(comp);
	}

	bc::barstruct getConstr()
	{
		bc::barstruct constr;
		constr.createBinaryMasks = true;
		constr.createGraph = false;
		constr.attachMode = bc::AttachMode::morePointsEatLow;
		//constr.attachMode = bc::AttachMode::closer;
		constr.returnType = bc::ReturnType::barcode2d;

		constr.addStructure(type, bc::ColorType::native, bc::ComponentType::Component);

		return constr;
	}
};

void moveMlToRef(MLSettings& in, RefSettings& out)
{
	for (size_t i = 0; i < in.values.size(); i++)
	{
		OptionValue& set = in.values[i];

		const BackString& label = set.name;
		switch (set.type)
		{
		case OptionValue::sv_bool:
			out.add(SettingValue(label, set.data.b));
			break;
		case OptionValue::sv_int:
			out.add(SettingValue(label, set.data.i));
			break;
		case OptionValue::sv_double:
			out.add(SettingValue(label, set.data.d));
			break;
		case OptionValue::sv_str:
			out.add(SettingValue(label, *set.data.s));
			break;
		case OptionValue::sv_enum:
		{
			SettingValue enu(label, set.data.e->selected, {});
			for (size_t i = 0; i < set.data.e->values.size(); i++)
			{
				enu.data.e->enums.push_back({set.data.e->names[i], set.data.e->values[i]});
			}

			out.add(enu);
			break;
		}
		case OptionValue::sv_path:
			assert(false);
			break;
		}
	}
}


class DatasetClassificationBlock : public IBlock
{
	BackPathStr filesRoot = "/Users/sam/Edu/datasets/hirise-map-proj-v3/map-proj-v3/";

public:
	BarSettings bar;
	LandClassifier cluster;
	DatasetClassificationBlock()
	{
		IBlock::settings = {
			{"Path", filesRoot, false}
		};
		bar.adjustSettings(IBlock::settings);
		moveMlToRef(cluster.settings, IBlock::settings);
	}

	const BackString name() const override
	{
		return "Dataset Classification";
	}

	RetLayers execute(InOutLayer iol) override
	{
		// Enumerate each file in filesRoot

		Project* proj = Project::getProject();


		ConvertCollection landscapes;
		bc::BarcodeCreator bcc;
		std::vector<BackString> paths;
		for (const auto & entry : fs::directory_iterator(filesRoot))
		{
			BackImage main = imread(entry.path());
			paths.push_back(entry.path());

			bc::barstruct constr = bar.getConstr();

			CachedBaritemHolder cache;
			cache.create(&main, constr, nullptr);

			landscapes.addItem(cache.getRoot());
		}
		// cluster.perform();
		cluster.predict(landscapes);
		for (size_t i = 0; i < paths.size(); i++)
		{
			std::cout << paths[i] << " -> " << cluster.test(i) << std::endl;
		}

		return {};
	}

	void clear() override
	{
		// cluster.clear();
	}

	void addInput(InOutLayer iol) override
	{
		// Project* proj = Project::getProject();

		// proj->saveProject();
	}
};

BlockRegister<DatasetClassificationBlock> DatasetClassificationBlockReg;
