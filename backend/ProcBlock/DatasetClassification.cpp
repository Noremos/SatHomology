#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include <filesystem>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include "Common.h"

namespace fs = std::filesystem;

import RasterLayers;
import IBlock;
import LayersCore;
import ProjectModule;
import AlgUtils;
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
		constr.createGraph = true;
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
	// BackPathStr filesRoot = "/Users/sam/Edu/papers/landscape/set/";
	int limit = 150;
	int resolution = 1;

public:
	BarSettings bar;
	LandClassifier cluster;
	DatasetClassificationBlock()
	{
		IBlock::settings = {
			{"Path", filesRoot, false},
			// {"Limit", limit},
			{"Resolution", resolution}
		};
		bar.adjustSettings(IBlock::settings);
		moveMlToRef(cluster.settings, IBlock::settings);
	}

	const BackString name() const override
	{
		return "Dataset Classification";
	}

	RetLayers execute(InOutLayer) override
	{
		// Enumerate each file in filesRoot

		ConvertCollection landscapes;
		*landscapes.settings.getInt("Resolution") = resolution;
		bc::BarcodeCreator bcc;
		if (limit <= 0)
			limit = INT_MAX;


		std::ifstream res("/Users/sam/Edu/datasets/hirise-map-proj-v3/labels-map-proj-v3.txt");
		// Read file line by line

		std::unordered_map<std::string, int> sourceFiles;
		std::string line;
		int counter[7] = { 0, 0, 0, 0, 0, 0, 0 };

		while (std::getline(res, line))
		{
			int p = line.find_last_of(' ');
			std::string name = line.substr(0, p);
			int id = std::stoi(line.substr(p + 1));
			counter[id]++;
			sourceFiles.insert(std::pair(name, id));
		}


		for (size_t i = 0; i < 7; i++)
		{
			std::cout << i + 1 << ": " << counter[i] << std::endl;
		}

		int maxAllowed = *std::min_element(counter, counter + 7);
		std::fill_n(counter, 7, 0);


		maxAllowed = 50;
		std::cout << "Sample " << maxAllowed << " elements from each cluster" << std::endl;

		landscapes.performOnPerform();

		int added = 0;
		std::vector<BackString> names;
		for (auto& entry : sourceFiles)
		{
			int correctId = entry.second;
			if (counter[correctId] > maxAllowed)
				continue;

			BackString path = filesRoot / entry.first;
			// assert(pathExists(path));

			counter[correctId]++;

			BackImage main = imread(path);

			bc::barstruct constr = bar.getConstr();

			CachedBaritemHolder cache;
			cache.create(&main, constr, nullptr);

			landscapes.addAllLines(cache);

			names.push_back(entry.first);
			added++;
			// if (i++ >= limit)
			// 	break;
		}

		// cluster.perform();
		cluster.predict(landscapes);
		int results[7] = { 0, 0, 0, 0, 0, 0, 0 };
		std::fill_n(results, 7, 0);

		int correctCount = 0;
		for (size_t i = 0; i < added; i++)
		{
			int prediction = cluster.test(i);
			int correctId = sourceFiles[names[i]];
			bool correct = prediction == correctId;
			if (correct)
			{
				results[correctId]++;
				correctCount++;
			}
			// std::cout << paths[i] << " -> " << cluster.test(i);
			// if (correct)
			// {
			// 	std::cout << " (correct)";
			// }
			// else
			// {
			// 	std::cout << " (incorrect, " << prediction << " vs " << correctId << ")";
			// }
			// std::cout << std::endl;
		}

		std::cout << "Correct: " << correctCount << "/" << added << " (" << correctCount * 100.0 / added << "%)" << std::endl;
		for (size_t i = 0; i < 7; i++)
		{
			std::cout << i + 1 << ": " << results[i] << "/" << counter[i] << std::endl;
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
