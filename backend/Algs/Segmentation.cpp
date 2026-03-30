#include <vector>
#include <memory>
#include "Barcode/PrjBarlib/include/barline.h"
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include <random>
#include <limits>


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


inline constexpr std::string_view PATH = "/Users/sam/Edu/datasets/Semantic segmentation dataset";


std::unordered_map<uint32_t, uint16_t> colorToIdMap;
struct SegmentationData
{
	uint32_t maxClassed = 0;
	std::array<uint32_t, 256> classPixels{};
	std::array<uint32_t, 256> predictedPixels{};
	std::array<uint32_t, 256> truePositive{};
	uint32_t totalPixels{};


	using Pixels = std::vector<uint32_t>;
	std::vector<Pixels> objects;
};

std::vector<std::string> getImageFiles(const std::string& dirPath)
{
	std::vector<std::string> files;
	for (const auto& entry : std::filesystem::directory_iterator(dirPath))
	{
		if (entry.is_regular_file())
		{
			std::string filename = entry.path().filename().string();
			if (filename.ends_with(".jpg") || filename.ends_with(".png"))
			{
				files.push_back(filename);
			}
		}
	}
	std::sort(files.begin(), files.end());
	return files;
}

void validateSegmentation(const bc::barstruct& constr)
{
	std::string basePath(PATH.data(), PATH.size());

	auto tiles = getImageFiles(basePath);

	SegmentationData segData;

	for (const auto& tile : tiles)
	{
		std::string tilePath = basePath + "/" + tile;
		std::string imagesPath = tilePath + "/images/";
		std::string masksPath = tilePath + "/masks/";

		auto imageFiles = getImageFiles(imagesPath);

		for (const auto& imgFile : imageFiles)
		{
			BackImage srcImg = imread(imagesPath + imgFile);
			BackImage maskImg = imread(masksPath + imgFile.substr(0, imgFile.find_last_of('.')) + ".png");

			if (srcImg.width() == 0 || maskImg.width() == 0)
				continue;

			for (int y = 0; y < srcImg.height(); ++y)
			{
				for (int x = 0; x < srcImg.width(); ++x)
				{
					auto trueClass = maskImg.get(x, y);
					uint id = 0;
					auto it = colorToIdMap.find(trueClass.getInt());
					if (it == colorToIdMap.end())
					{
						colorToIdMap[trueClass.getInt()] = id = segData.maxClassed++;
						segData.objects.push_back({});
					}
					else
						id = it->second;

					segData.objects[id].push_back(y * srcImg.width() + x);
					segData.classPixels[id]++;
					segData.totalPixels++;
				}
			}


			int threshhold = std::numeric_limits<int>::max();
			for (size_t i = 0; i < segData.objects.size(); i++)
			{
				assert(segData.objects.size() > 0);
				if (segData.objects.size() < threshhold)
					threshhold = segData.objects.size();
			}


			auto barcode = bc::BarcodeCreator::create(srcImg, constr);

			for (size_t i = 0; i < barcode->barlines.size(); i++)
			{
				auto* line = barcode->barlines[i];
				if (line->matr.size() < threshhold)
					continue;
			}

		}
	}

	double mIoU = 0.0;
	for (size_t i = 0; i < 256; ++i)
	{
		if (segData.classPixels[i] == 0) continue;
		double iou = static_cast<double>(segData.truePositive[i]) /
					(segData.classPixels[i] + segData.predictedPixels[i] - segData.truePositive[i]);
		mIoU += iou;
	}

	std::cout << "Mean IoU: " << mIoU << "\n";
}