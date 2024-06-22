#include <filesystem>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include <limits>
// #include <map>

#include "Common.h"
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "../Interfaces/IBlock.h"

#include "../Core/RefSettings.h"
#include "../MatrImg.h"
#include "../CachedBarcode.h"
#include "../MLSettings.h"

#include "Dataset/PointClass.h"
#include "Dataset/SelfClassCluster.h"
#include "Dataset/DatasetWork.h"

namespace fs = std::filesystem;
using namespace std::string_literals;

import RasterLayers;
// import IBlock;
import LayersCore;
// import ProjectModule;
import AlgUtils;
// import MatrModule;
// import ConvertItem;
// import CachedBarcode;
// import MLSettings;
// import RefSettings;

class BarSettings
{
public:
	bc::ProcType type;
	void adjustSettings(RefSettings& settings)
	{
		SettingValue comp("type"s, type,
		{
			{"От 0 до 255"s, bc::ProcType::f0t255},
			{"От 255 до 0"s, bc::ProcType::f255t0},
			{"По расстоянию"s, bc::ProcType::Radius},
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

		constr.addStructure(type, bc::ColorType::gray, bc::ComponentType::Component);

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

class WriterProcessor
{
	StateBinFile::BinStateWriter writer;
public:
	WriterProcessor(int resolution)
	{
		writer.open("out.bin");
		writer.pInt(resolution);
	}

	int addToSet(ConvertClass* item, int classId)
	{
		// Save to disk --------------- --------------- ---------------
		writer.pBool(true);
		// writer.pString(entry.first); // TODO: Use conbext instead of classId
		writer.pInt(classId);
		saveToDist(writer, item);

		// std::vector<float> temp;
		// item->getSignatureAsVector(temp);
		// saveToDist(writer, temp);
		// Save to disk end --------------- --------------- ---------------
		return 0;
	}

	void predict()
	{
		writer.pBool(false);
		writer.close();
	}


	int test(int id)
	{
		return 0;
	}

	void setClasses(int n)
	{
		writer.pShort(n);
	}

	void saveToDist(StateBinFile::BinStateWriter& out, ConvertClass* item)
	{
		auto& lines = item->pathset;
		out.pArray(static_cast<buint>(lines.size()));
		for(auto& line : lines)
		{
			out.pArray(static_cast<buint>(line.size()));
			for (size_t i = 0; i < line.size(); i++)
			{
				out.pInt(line[i].x);
				out.pInt(line[i].y);
			}
		}
	}

	void saveToDist(StateBinFile::BinStateWriter& out, std::vector<float>& set)
	{
		out.pArray(static_cast<buint>(set.size()));
		for(auto& hei : set)
		{
			out.pFloat(hei);
		}
	}
};


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


		landscapes.performOnPerform();

		SignatureProcessor<SelfCluster, SignatureType::Iter> iterSelfCuster(resolution);
		SignatureProcessor<SelfCluster, SignatureType::Combined> combinedSelfCuster(resolution);
		SignatureProcessor<SelfCluster, SignatureType::CombinedIter> combinedIterCuster(resolution);

		SignatureProcessor<LandClassifier, SignatureType::Iter> iterLandCuster(cluster, resolution);
		SignatureProcessor<LandClassifier, SignatureType::Combined> combinedLandCuster(cluster, resolution);
		SignatureProcessor<LandClassifier, SignatureType::CombinedIter> combinedIterLandCuster(cluster, resolution);

		PointsProcessor<PointCluster> pointCuster;

		bc::barstruct constr = bar.getConstr();
		constr.createGraph = false; // Do not create empty nodes
		int maxAllowed = 100;

		DatasetWork dw;
		// dw.open(filesRoot);
		// dw.openCraters("ctaters");
		// dw.openCraters("ctx_samv1");
		// dw.openCraters("ctx_samv2/train");
		dw.openCraters("planet/train", "Earth", "Moon");
		// dw.openCraters("test_dataset");
		// dw.openCraters("ctx_samv2/valid");
		// dw.open();
		// std::cout << "iterSelfCuster" << std::endl;
		// dw.predict(maxAllowed, landscapes, constr, iterSelfCuster); // Ok, но почему0то много кластеров
		// std::cout << "combinedSelfCuster" << std::endl;
		// dw.predict(maxAllowed, landscapes, constr, combinedSelfCuster);
		// std::cout << "combinedIterCuster" << std::endl;
		// dw.predict(maxAllowed, landscapes, constr, combinedIterCuster);

		// std::cout << "pointCuster" << std::endl;
		// dw.predict(maxAllowed, landscapes, constr, pointCuster); // Broken

		std::cout << "iterLandCuster" << std::endl;
		dw.predict(maxAllowed, landscapes, constr, iterLandCuster);
		std::cout << "combinedLandCuster" << std::endl;
		// dw.predict(maxAllowed, landscapes, constr, combinedLandCuster);
		// std::cout << "combinedIterLandCuster" << std::endl;
		// dw.predict(maxAllowed, landscapes, constr, combinedIterLandCuster);

		// WriterProcessor writer(resolution);
		// dw.predict(maxAllowed, landscapes, constr, writer);

		std::cout << "Done" << std::endl;
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
