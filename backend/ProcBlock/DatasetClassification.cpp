#include <filesystem>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include <limits>
#include <string_view>
// #include <map>

#include "Common.h"
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "../Interfaces/IBlock.h"

#include "../Core/RefSettings.h"
#include "../MatrImg.h"
#include "../CachedBarcode.h"
#include "../MLSettings.h"

// #include "Dataset/PointClass.h"
// #include "Dataset/SelfClassCluster.h"
#include "Dataset/LandscapeCalss.h"
#include "Dataset/DatasetWork.h"
#include "../Layers/layerInterface.h"
#include "../Layers/Rasterlayers.h"

namespace fs = std::filesystem;
using namespace std::string_literals;

// import RasterLayers;
// import IBlock;
// import LayersCore;
// //import IAlgorithm
#include "../backend/project.h"
//import AlgUtils;
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
	void clear()
	{

	}

	int addToSet(LandscapeClass* item, int classId)
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

	void saveToDist(StateBinFile::BinStateWriter& out, LandscapeClass* item)
	{
		auto& lines = item->landscape;
		out.pArray(static_cast<buint>(lines.size()));
		for(LyambdaLine& line : lines)
		{
			out.pArray(static_cast<buint>(line.size()));
			for (size_t i = 0; i < line.size(); i++)
			{
				out.pFloat(line.points[i].x);
				out.pFloat(line.points[i].y);
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


class BarWrtierProc : BarWriter
{
	StateBinFile::BinStateWriter writer;
public:
	BarWrtierProc()
	{
		writer.open("outitem.bin");
	}

	int addToSet(LandscapeClass* item, int classId)
	{
		return 0;
	}

	// void add(const CachedBarcode& cache)
	// {
	// 	// Save to disk --------------- --------------- ---------------
	// 	writer.pBool(true);
	// 	cache.
	// 	// writer.pString(entry.first); // TODO: Use conbext instead of classId
	// 	writer.pInt(classId);
	// 	saveToDist(writer, item);

	// 	// std::vector<float> temp;
	// 	// item->getSignatureAsVector(temp);
	// 	// saveToDist(writer, temp);
	// 	// Save to disk end --------------- --------------- ---------------
	// 	return 0;
	// }

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

	void saveToDist(StateBinFile::BinStateWriter& out, LandscapeClass* item)
	{
		auto& lines = item->landscape;
		out.pArray(static_cast<buint>(lines.size()));
		for(LyambdaLine& line : lines)
		{
			out.pArray(static_cast<buint>(line.size()));
			for (size_t i = 0; i < line.size(); i++)
			{
				out.pInt(line.points[i].x);
				out.pInt(line.points[i].y);
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


class TestIterator
{
public:
	virtual void restart() = 0;
	virtual void iterate() = 0;

	virtual bool ended() = 0;

	virtual bool needRebuild()
	{
		return false;
	}

	virtual void print(BackString& out) = 0;
};


class FuncIterator : public TestIterator
{
	int& funcId;
	int id = 0;
	std::vector<int> funcs = { 0, 1, 2, 3, 0};
public:

	FuncIterator(int& funcId) : funcId(funcId)
	{ }

	void restart() override
	{
		id = 0;
		funcId = funcs[id];
	}

	void iterate() override
	{
		id++;
		funcId = funcs[id];
	}

	bool ended() override
	{
		return id >= funcs.size() - 1;
	}

	void print(BackString& out) override
	{
		out += "Func: "s + to_string(funcId) + " ";
	}
};

class ProcTypeIterator : public TestIterator
{
	bc::ProcType& type;
	bool end = false;
public:

	ProcTypeIterator(bc::ProcType& intype) : type(intype)
	{ }

	void restart() override
	{
		type = bc::ProcType::Radius;
	}

	void iterate() override
	{
		if (type == bc::ProcType::f255t0)
			end = true;
		type = bc::ProcType::f255t0;
	}

	bool needRebuild() override
	{
		return true;
	}

	bool ended() override
	{
		return end;
	}

	void print(BackString& out) override
	{
		out += "ProcType: "s + (type == bc::ProcType::f0t255 ? "f0t255 " : "f255t0 ");
	}
};


class StepIterator : public TestIterator
{
	int stepId;
	float& step;
	std::vector<float> steps = { 1.0f, 1.f};
public:

	StepIterator(float& step) : step(step)
	{ }

	void restart() override
	{
		stepId = 0;
		step = steps[0];
	}

	void iterate() override
	{
		++stepId;
		step = steps[stepId];
	}

	bool ended() override
	{
		return stepId >= steps.size() - 1;
	}


	void print(BackString& out) override
	{
		out += "Step: "s + to_string(step) + " ";
	}
};

using namespace std::literals::string_view_literals;

class FuncNameIterator : public TestIterator
{
	int stepId = 0;
	const char*& methodName;
	std::vector<BackStringView> funcName =
	{
		"hierarchical"sv,
		"mds_kmeans"sv,

		// "spectral"sv,
		// "dbscan"sv,
		// "affinity"sv,
		""sv
	};

public:

	FuncNameIterator(const char*& ref) : methodName(ref)
	{
		restart();
	}

	void restart() override
	{
		stepId = 0;
		methodName = funcName[stepId].data();
	}

	void iterate() override
	{
		++stepId;
		methodName = funcName[stepId].data();
	}

	bool ended() override
	{
		return stepId >= funcName.size() - 1; // 1 for the last empty string and 1 to get last valid index
	}


	void print(BackString& out) override
	{
		out += "Method: "s;
		out += funcName[stepId];
		out += " "s;
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
		return "Landscape Clusterization";
	}



	void saveDebug(const bc::barstruct& constr, LandscapeCollection& landscapes)
	{
		// **** Binary writer ****
		WriterProcessor writer(resolution);
		// dw.collect(maxAllowed, landscapes, constr, writer);
		CachedBaritemHolder cache;

		BackImage main = imread("/Users/sam/Edu/bar/test2/s4.png"s, true);
		cache.create(&main, constr, nullptr);
		// cache.saveLoadState(&writer);

		landscapes.addAllLines(cache);
		LandscapeClass* item = landscapes.back();

		// Fitting --------------- ---------------

		writer.setClasses(1);
		writer.addToSet(item, 0);
		writer.predict();
	}

	void expereemnt2calsses(LandscapeCollection& landscapes, bc::barstruct& constr, PointProcessor<LandscapeCluster>& landPyCluste)
	{
		int maxAllowed = 100;

		DatasetWork dw;
		// dw.open(maxAllowed);
		// dw.openCraters("craters/ctx_samv2/train");
		// dw.openCraters("objects/eurosat", {"Forest", "Pasture"});
		// dw.openCraters("objects/256_ObjectCategories", {"009.bear", "007.bat"});

		dw.openCraters(maxAllowed, "objects/NWPU-RESISC45", {"intersection", "harbor"});
		// dw.openCraters(maxAllowed, "objects/NWPU-RESISC45");
		// dw.openCraters("objects/UCMerced_LandUse/Images", {"forest", "parkinglot"});
		// dw.openCraters("test_dataset");


		// **** Cluster ****

		// std::cout << "Start" << std::endl;
		dw.collect(maxAllowed, landscapes, constr, landPyCluste);
		// landPyCluste.dummy.curFUnc = 0;
		dw.predict(landPyCluste);
	}

	void experement4classes(LandscapeCollection& landscapes, bc::barstruct& constr, PointProcessor<LandscapeCluster>& landPyCluste)
	{
		int maxAllowed = 100;

		// **** Work ****

		DatasetWork dws;
		dws.openCraters(maxAllowed, "objects/NWPU-RESISC45", {"desert", "intersection", "railway", "commercial_area"});
		dws.collect(maxAllowed, landscapes, constr, landPyCluste);

		// **** Iterators ****
		FuncIterator iterFunc(landPyCluste.dummy.curFUnc);
		StepIterator iterStep(landPyCluste.dummy.iterationStep);
		FuncNameIterator iterFuncName(landPyCluste.dummy.methodName);
		ProcTypeIterator iterType(constr.proctype);
		iterType.restart();
		iterFunc.restart();
		iterStep.restart();
		iterFuncName.restart();

		// **** Collect ****
		std::vector<TestIterator*> iters {&iterFunc, &iterStep, &iterFuncName, &iterType};

		BackFileWriter result("result.txt", std::ios::trunc);
		while (true)
		{
			TestIterator& it = *iters[0];

			// print
			BackString out;
			for (auto* t : iters)
			{
				t->print(out);
				out += "| ";
			}
			cout << out << endl;

			auto res = dws.predict(landPyCluste);
			result << out << " => " << res.first << "/" << res.second << ", " << ((100.f * res.first) / res.second) << "%" << endl;


			int i = 0;
			for (;i < iters.size(); i++)
			{
				bool allowMoveNext = true;
				iters[i]->iterate(); // Switch next
				if (iters[i]->ended())
				{
					iters[i]->restart(); // Rstart and go iterate next
				}
				else
				{
					if (iters[i]->needRebuild())
					{
						landPyCluste.clear();
						dws.collect(maxAllowed, landscapes, constr, landPyCluste);
					}
					allowMoveNext = false;
				}

				if (!allowMoveNext)
					break;
			}

			if (i == iters.size())
				break;
		}


		cout << "DONE" << endl;
	}

	RetLayers execute(InOutLayer) override
	{
		// Enumerate each file in filesRoot

		LandscapeCollection landscapes;
		*landscapes.settings.getInt("Resolution") = resolution;
		bc::BarcodeCreator bcc;
		if (limit <= 0)
			limit = INT_MAX;


		landscapes.performOnPerform();
		landscapes.round = false;

		// ClassProcessor classer;

		bc::barstruct constr = bar.getConstr();
		constr.createGraph = false; // Do not create empty nodes
		// constr.proctype = bc::ProcType::Radius;

		// saveDebug(constr, landscapes);
		// return {};

		PointProcessor<LandscapeCluster> landPyCluste;

		// **** Work ****
		experement4classes(landscapes, constr, landPyCluste);
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


private:
};



BlockRegister<DatasetClassificationBlock> DatasetClassificationBlockReg;
