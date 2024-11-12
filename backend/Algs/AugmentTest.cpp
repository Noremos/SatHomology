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
RetLayers exeAugment(InOutLayer iol, const MLSettings& setting)
{
	bc::barstruct constr = getConstr(setting);
		constr.coltype = bc::ColorType::gray;
		constr.proctype = bc::ProcType::Radius;
	// {
	// 	BackImage a = imread("/Users/sam/Edu/datasets/objects/eurosat_augment/Highway_1185.png"s);
	// 	// a = a.getRect(0, 11, 16, 12);
	// 	// BackImage a = imread("/Users/sam/Edu/bar/base/ident-ss1.png"s, true);



	// 	auto baseImg = bc::BarcodeCreator::create(a, constr);
	// 	describe(baseImg.get());

	// 	BackImage b = a.mirror();
	// 	auto augImg = bc::BarcodeCreator::create(b, constr);
	// 	describe(augImg.get());

	// 	float res = baseImg->compireBestRes(augImg.get(), bc::CompireStrategy::CommonToLen);
	// 	cout << res << endl;
	// }
	// return {};

	RetLayers ret;
	BackImage src;

	// std::unique_ptr<bc::Baritem> containner(bcc.run(&src, constr, dummy));
	// bc::Baritem* item = containner.get();

	std::unordered_map<BackString, std::unique_ptr<ImageC>> sourceFiles;
	int maxAllowed = 10;
	BackPathStr srcleanb("/Users/sam/Edu/datasets/objects/eurosat_augment");
	for (const auto& entry : std::filesystem::directory_iterator(srcleanb))
	{
		auto path = entry.path();
		auto fileName = path.filename().string();

		// int number_.pos = fileName.find_first_of('_');
		int subtype_pos = fileName.find_first_of('-');
		int ext_type = fileName.find_last_of('.');

		string ext = fileName.substr(ext_type + 1);
		if (ext != "png" && ext != "jpg")
		{
			continue;
		}

		int lastPos = subtype_pos == std::string::npos ? ext_type : subtype_pos;
		BackString onlyName = fileName.substr(0, lastPos);
		auto& img = sourceFiles[onlyName];
		// std::cout << onlyName << endl;
		if (img == nullptr)
		{
			img.reset(new ImageC);
		}

		if (subtype_pos == std::string::npos)
		{
			img->name = fileName;
		}
		else
		{
			BackString sub_type = fileName.substr(subtype_pos + 1, ext_type - subtype_pos - 1);
			if (augmentNumber.count(sub_type) == 0)
			{
				augmentNumber.insert(std::pair(sub_type, augmentNumber.size()));
			}

			int sub_type_pos = augmentNumber[sub_type];
			img->augments[sub_type_pos] = fileName;
		}
	}

	std::vector<Results> numToNameMap(augmentNumber.size());
	for (auto augName : augmentNumber)
	{
		numToNameMap[augName.second] = augName.first;
	}

	int augSize = numToNameMap.size();

	for (auto& filePair : sourceFiles)
	{
		ImageC& img = *(filePair.second.get());
		BackImage src = imread(srcleanb / img.name);
		auto baseImg = bc::BarcodeCreator::create(src, constr);
		// baseImg->normalize();

		for (int i = 0; i < augSize; i++)
		{
			assert(!img.augments[i].empty());

			BackImage aug = imread(srcleanb / img.augments[i]);
			auto augImg = bc::BarcodeCreator::create(aug, constr);
			// augImg->normalize();

			float res = baseImg->compireFull(augImg.get(), bc::CompireStrategy::CommonToLen);
			numToNameMap[i].add(res);
		}
	}

	for (auto& res : numToNameMap)
	{
		res.printResult();
	}

	return {};
}

static AlgFuncRegister registerAugment("AugmentText", exeAugment, mkSettingsType);
// class AutoRun
// {
// public:
// 	AutoRun()
// 	{
// 		exeAugment({}, mkSettingsType());
// 	}
// };
// static AutoRun autoee;
