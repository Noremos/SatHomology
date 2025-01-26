#include <vector>
#include <memory>
#include "Barcode/PrjBarlib/include/barline.h"
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include <random>


#include "../MatrImg.h"
#include "../MLSettings.h"
#include "../Layers/layerInterface.h"
#include "../Layers/Rasterlayers.h"

// import LayersCore;
// import RasterLayers;
//import BackBind;
//import IAlgorithm
#include "../backend/project.h"
// import MLSettings;
// import MatrModule;
//import IAlgorithm
#include "../backend/Interfaces/IAlgorithm.h"
//import AlgUtils;
//import SimpleBar;
#include "../backend/Algs/SimpleBar.h"
#include "../backend/ProcBlock/Dataset/DatasetWork.h"

// Linear interpolation function


struct ImageC
{
	BackString name;
	std::array<BackString, 10> augments;
};


class  Results
{
public:
	Results(BackStringView augName = "") : augName(augName) {}

	Results& operator= (const BackString& augName)
	{
		this->augName = augName;
		return *this;
	}

	void add(float result)
	{
		sumOfCmp += result;
		++total;
	}

	void printResult() const
	{
		std::cout << augName << " " << sumOfCmp / total << std::endl;
	}

private:
	BackString augName;

	float sumOfCmp = 0;
	int total = 0;

};

std::unordered_map<BackString, int> augmentNumber;


void describe(bc::Baritem* itm)
{
	cout << "BARLINE" << endl;
	for (auto& line : itm->barlines)
	{
		cout << line->start.getAvgFloat() << " : " << line->end().getAvgFloat() << endl;
		// for (size_t i = 0; i < line->matr.size(); ++i)
		// {
		// 	auto& m = line->matr[i];
		// 	cout << m.getX() << " " << m.getY() << " : " << m.value.getAvgFloat() <<  endl;;
		// }
	}
}

void save(BackStringView fileName, bool normilize)
{
	bc::barstruct constr;
	constr.coltype = bc::ColorType::gray;
	constr.proctype = bc::ProcType::Radius;

	BackPathStr srcleanb("/Users/sam/Edu/datasets/objects/eurosat_augment");
	string fullname(fileName.data(), fileName.length());
	fullname += ".png"sv;

	string outName(fileName.data(), fileName.length());
	outName += "gray.png";
	imwrite(srcleanb / outName, imread(srcleanb / fullname));
	exit(0);


	BackImage img = imread(srcleanb / fullname);
	auto baseImg = bc::BarcodeCreator::create(img, constr);
	// baseImg->sortByLen();
	if (normilize)
	{
		outName += "-norm.bin";
		baseImg->normalize();
		StateBinFile::BinStateWriter writer;
		writer.open(outName);
		auto count = writer.pInt(baseImg->barlines.size());
		for (int i = 0; i < count; ++i)
		{
			auto line = baseImg->barlines[i];
			writer.pFloat(line->start.getAvgFloat());
			writer.pFloat(line->end().getAvgFloat());
		}
		writer.close();
	}
	else
	{
		outName += ".bin";

		StateBinFile::BinStateWriter writer;
		writer.open(outName);
		auto count = writer.pInt(baseImg->barlines.size());
		for (int i = 0; i < count; ++i)
		{
			auto line = baseImg->barlines[i];
			writer.pShort(line->start.getAvgUchar());
			writer.pShort(line->end().getAvgUchar());
		}
		writer.close();
	}
}

void buildSave(BackString path, BackString outName, bc::ProcType type = bc::ProcType::Radius)
{
	bc::barstruct constr;
	constr.coltype = bc::ColorType::gray;
	constr.proctype = type;

	BackPathStr srcleanb(path);
	BackImage img = imread(srcleanb);
	auto baseImg = bc::BarcodeCreator::create(img, constr);

	StateBinFile::BinStateWriter writer;
	writer.open(outName);

	writer.pInt(baseImg->barlines.size());
	for (auto* line : baseImg->barlines)
	{
		writer.pInt(line->start.getAvgUchar());
		writer.pInt(line->m_end.getAvgUchar());
		writer.pInt(line->matr.size());
		for (auto& a : line->matr)
		{
			writer.pShort(a.getX());
			writer.pShort(a.getY());
		}
	}
}

RetLayers exeAugment(InOutLayer iol, const MLSettings& setting)
{
	bc::barstruct constr;
	constr.coltype = bc::ColorType::gray;
	constr.proctype = bc::ProcType::f0t255;

	BackImage img = imread(BackPathStr("input1.jpg"));
	auto baseImg = bc::BarcodeCreator::create(img, constr);
	std::cout << baseImg->barlines.size() << endl;

	// buildSave("python_extra/inputs/input2-smlf.png", "input2-sml-radius.bin");
	// buildSave("input2-sml.jpg", "input2-sml-f255jpg.bin", bc::ProcType::f255t0);
	// buildSave("input2-smlf.png", "input2-sml-f255.bin", bc::ProcType::f255t0);
	// buildSave("input1-smlf.png", "input1-sml-f0.bin", bc::ProcType::f0t255);
	buildSave("input1-sml.jpg", "input1-sml-f0jpg.bin", bc::ProcType::f0t255);

	// buildSave("python_extra/inputs/input2.jpg", "input2-radius.bin");
	// buildSave("python_extra/inputs/input2.jpg", "input2-f255.bin", bc::ProcType::f255t0);

	// buildSave("python_extra/inputs/input1.jpg", "input1-radius.bin");
	// buildSave("python_extra/inputs/input1.jpg", "input1-f255.bin", bc::ProcType::f255t0);
	// save("HerbaceousVegetation_170", true);

	// save("HerbaceousVegetation_170", false);
	// save("HerbaceousVegetation_170-blur", false);
	// save("HerbaceousVegetation_170-bright", false);
	// save("HerbaceousVegetation_170-distort-rt", false);
	// save("HerbaceousVegetation_170-distort", false);
	// save("HerbaceousVegetation_170-gauss-rot", false);
	// save("HerbaceousVegetation_170-gauss", false);
	// save("HerbaceousVegetation_170-hsv", false);
	// save("HerbaceousVegetation_170-med-blur", false);
	// save("HerbaceousVegetation_170-mirror", false);



	return {};
}

static AlgFuncRegister registerAugment("AugmentText", exeAugment, mkSettingsType);
class AutoRun
{
public:
	AutoRun()
	{
		exeAugment({}, mkSettingsType());
	}
};
static AutoRun autoee;
