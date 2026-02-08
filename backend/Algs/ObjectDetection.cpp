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
#include "../Algs/Gen2/Trainer.h"


inline constexpr std::string_view PATH = "/Users/sam/Edu/datasets/SkyFusion/";

struct Range
{
	int min;
	int max;
};

struct Rect
{
	Range x;
	Range y;
};

Rect findCountur(const bc::barvector& matr)
{
	// Compute bounding contour (axis-aligned) for this barline and draw it
	int minX = std::numeric_limits<int>::max();
	int minY = std::numeric_limits<int>::max();
	int maxX = std::numeric_limits<int>::min();
	int maxY = std::numeric_limits<int>::min();
	for (const auto& pm : matr)
	{
		minX = std::min(minX, pm.getX());
		minY = std::min(minY, pm.getY());
		maxX = std::max(maxX, pm.getX());
		maxY = std::max(maxY, pm.getY());
	}

	return Rect{ Range{minX, maxX}, Range{minY, maxY} };
}


void drawCountur(BackImage& out, const Rect rect, const Barscalar& color, int16_t width = 1)
{
	for (int x = rect.x.min; x < rect.x.max; ++x)
	{
		for (int16_t i = 0; i < width; i++)
		{
			out.setIfExists(x + i, rect.y.min, color);
			out.setIfExists(x + i, rect.y.max, color);
		}
	}
	for (int y = rect.y.min; y < rect.y.max; ++y)
	{
		for (int16_t i = 0; i < width; i++)
		{
			out.setIfExists(rect.x.min, y + i, color);
			out.setIfExists(rect.x.max, y + i, color);
		}
	}
}


// Draw a small percentage text like "<N>%" directly onto BackImage without
// using any external text libraries. Uses a simple 3x5 pixel font scaled by
// 'scale'.
void drawPercentText(BackImage& out, int percent, Barscalar color, int x = 0, int y = 0, int scale = 1)
{
	if (scale <= 0) scale = 1;
	// Clamp percent to 0..999 for display
	if (percent < 0) percent = 0;
	if (percent > 999) percent = 999;

	// Simple 3x5 font for '0'..'9' and '%'. Each entry contains 5 rows of 3 bits
	// row bit 0 is leftmost.
	static const uint8_t fontDigits[10][5] = {
		{0b111,0b101,0b101,0b101,0b111}, // 0
		{0b010,0b110,0b010,0b010,0b111}, // 1
		{0b111,0b001,0b111,0b100,0b111}, // 2
		{0b111,0b001,0b111,0b001,0b111}, // 3
		{0b101,0b101,0b111,0b001,0b001}, // 4
		{0b111,0b100,0b111,0b001,0b111}, // 5
		{0b111,0b100,0b111,0b101,0b111}, // 6
		{0b111,0b001,0b001,0b001,0b001}, // 7
		{0b111,0b101,0b111,0b101,0b111}, // 8
		{0b111,0b101,0b111,0b001,0b111}  // 9
	};

	// '%' glyph (3x5) approximation
	static const uint8_t fontPercent[5] = {
		0b101,
		0b001,
		0b010,
		0b100,
		0b101
	};

	// Build string digits (in memory, not using std::to_string to keep it simple)
	char buf[4] = {0,0,0,0};
	int len = 0;
	int tmp = percent;
	if (tmp == 0)
	{
		buf[len++] = '0';
	}
	else
	{
		char rev[4];
		int rlen = 0;
		while (tmp > 0 && rlen < 3)
		{
			rev[rlen++] = char('0' + (tmp % 10));
			tmp /= 10;
		}
		// reverse into buf
		for (int i = rlen - 1; i >= 0; --i) buf[len++] = rev[i];
	}

	// total characters is digits + '%' sign
	int totalChars = len + 1;
	const int cw = 3; // char width
	const int ch = 5; // char height
	const int spacing = 1;

	for (int ci = 0; ci < totalChars; ++ci)
	{
		if (ci + 1 == totalChars)
			color = Barscalar(0, 0, 0); // percent sign in black

		const uint8_t* glyph = nullptr;
		uint8_t localGlyph[5];
		if (ci < len)
		{
			int d = buf[ci] - '0';
			glyph = fontDigits[d];
		}
		else
		{
			// percent
			for (int r = 0; r < ch; ++r)
				localGlyph[r] = fontPercent[r];

			glyph = localGlyph;
		}

		int charX = x + ci * (cw + spacing) * scale;
		// draw glyph scaled
		for (int row = 0; row < ch; ++row)
		{
			for (int col = 0; col < cw; ++col)
			{
				bool on = (glyph[row] >> (cw - 1 - col)) & 1;
				if (!on) continue;
				// fill scaled pixel block
				int px0 = charX + col * scale;
				int py0 = y + row * scale;
				for (int sx = 0; sx < scale; ++sx)
				{
					for (int sy = 0; sy < scale; ++sy)
					{
						auto nx = px0 + sx;
						auto ny = py0 + sy;
						out.setIfExists(nx, ny, color);
					}
				}
			}
		}
	}
}


const std::vector<std::string> TRAIN
{
	"0c70835db_png_jpg.rf.b33d308d41070899738a2cbc81696811.jpg",
	"0e79e9286_png_jpg.rf.b68f40355db238f87f9bc7950ef650a2.jpg",
	"140d04fd-dea7-4d46-bce2-e20f73e155da_0_0_jpg.rf.43231b4a5386e2b324cac997563f6229.jpg",
	"980b5831-43b7-4adb-9e4b-67d6cff3ef68_0_1060_jpg.rf.3963875ade6f27e756c222ba5d9facaa.jpg",
	"135fdc4c-6656-4176-9873-9f00c6918293_1060_1060_jpg.rf.7ab251bd04a18367fea7549d75fd3a93.jpg",
	"P2672__1-0__3000___600_png_jpg.rf.596ca436c207483c9e8afa3e2710fb94.jpg",
	"P2753__1-0__0___1200_png_jpg.rf.a7109280c463052b2e9672296282f090.jpg",
	"P2754__1-0__1800___1200_png_jpg.rf.b7a1685eeacf8bac3949219f434f836d.jpg"
};

const std::vector<std::string> TEST
{
	"4d53fbdae_png_jpg.rf.8bf1ac78a6bc54af93e853cc33067dde.jpg", // boat
	"3e321b8a-9504-45aa-82b4-16158e28e290_1060_1060_jpg.rf.362d727519b454eeac2b42fb35142d3e.jpg", // air
	"03f84930-e2be-4c19-9afc-0dc26d02538e_1060_0_jpg.rf.c8ac4f9068dde47da4ac0211f59e751b.jpg", // air
	"15691_png_jpg.rf.fa449ee7d1d79b9307e7c7effb376344.jpg", // veh
	"P1412__1-0__3000___0_png_jpg.rf.50630bd0bc29553f3c61b2b6a9c5ab48.jpg" // air
};

// struct Search
// {
// 	uint16_t key;
// 	uint32_t distance;
// };


using DistanceValue = uint32_t;
using PlusValue = int32_t;

struct Score
{
	std::array<int32_t, 3> hits;
	uint32_t count{};
};

using PlusValues = std::array<PlusValue, 3>;

struct Node
{
	std::array<std::unique_ptr<Node>, 256> children;
	PlusValues types{};
	uint16_t length{};

	std::pair<Node*, DistanceValue> findChild(const uint16_t key) const
	{
		if (children[key])
		{
			return {children[key].get(), 0};
		}

		for (uint16_t i = 1; i < std::min<uint16_t>(256 - key, key); ++i)
		{
			if (children[key + i])
			{
				return {children[key + i].get(), i};
			}

			if (children[key - i])
			{
				return {children[key - i].get(), i};
			}
		}

		return {nullptr, 0};
	}

	Score findInChild(const bc::barline* line) const
	{
		auto [node, distance] = findChild(line->len().getAvgUchar());
		if (!node)
		{
			return Score{{}, 1}; // Return zeros if node not found
		}

		// Start with current node's type counts
		Score result{node->getPlusValue(), 1};
		for (size_t i = 0; i < 3; i++)
		{
			if (result.hits[i] > 0)
			{
				result.hits[i] = 1;
			}
		}

		// Penalize by distance to encourage exact matches
		for (size_t i = 0; i < 3; i++)
		{
			result.hits[i] = std::max<int32_t>(result.hits[i] - distance, 0);
		}

		// Accumulate counts from all children (visiting as many branches as possible)
		for (uint16_t i = 0; i < line->getChildrenCount(); ++i)
		{
			auto child = line->getChild(i);
			auto childCounts = node->findInChild(child);

			// Accumulate: add child counts to result
			for (uint16_t t = 0; t < 3; ++t)
			{
				result.hits[t] += childCounts.hits[t];
			}
			result.count += childCounts.count;
		}

		return result;
	}

	std::pair<uint16_t, float> findType(const bc::barline* line) const
	{
		const Score plus = findInChild(line);

		// Find closer

		uint16_t minTypeId = 0;
		uint16_t maxTypeId = 0;
		for (size_t i = 0; i < 3; i++)
		{
			if (plus.hits[i] < plus.hits[minTypeId])
			{
				minTypeId = i;
			}
			if (plus.hits[i] > plus.hits[maxTypeId])
			{
				maxTypeId = i;
			}
		}

		auto diff = plus.hits[maxTypeId] - plus.hits[minTypeId];
		return {maxTypeId, float(plus.hits[maxTypeId]) / float(plus.count)};
	}

	PlusValues getPlusValue() const
	{
		// uint16_t maxTypeId = 0;
		// for (size_t i = 0; i < 3; i++)
		// {
		// 	if (types[i] > types[maxTypeId])
		// 	{
		// 		maxTypeId = i;
		// 	}
		// }

		// Distance output;
		// for (size_t i = 0; i < 3; i++)
		// {
		// 	output[i] = types[i] - types[maxTypeId];
		// }

		return types;
	}

	int add(const bc::barline* line, const uint16_t typeId)
	{
		this->length = line->len().getAvgUchar();
		++this->types[typeId];

		int score = 1;
		for (uint16_t i = 0; i < line->getChildrenCount(); ++i)
		{
			auto child = line->getChild(i);
			if (child)
			{
				const uint16_t key = child->len().getAvgUchar();
				if (!children[key])
				{
					children[key] = std::make_unique<Node>();
				}
				score += children[key]->add(child, typeId);
			}
		}

		return score;
	}
};


struct ClassicTrainer
{
	std::array<std::vector<std::unique_ptr<bc::Baritem>>, 3> constrs;


	void add(bc::Baritem* item, const uint16_t typeId)
	{
		constrs[typeId].emplace_back(item->clone());
	}

	std::pair<uint16_t, float> findType(const bc::Baritem* in) const
	{
		float maxRes = 0.0f;
		uint16_t maxTypeId = 0;
		for (size_t i = 0; i < 3; i++)
		{
			for (const auto& item : constrs[i])
			{
				float res = item->compareFull(in, bc::CompareStrategy::CommonToLen);
				if (res > maxRes)
				{
					maxRes = res;
					maxTypeId = static_cast<uint16_t>(i);
				}
			}
		}
		return {maxTypeId, maxRes};
	}
};
struct DatasetData
{
	int addItem(bc::Baritem* item, const uint16_t categoryId)
	{
		classic.add(item, categoryId);
		return root.add(item->getRootNode(), categoryId);
	}

	std::pair<uint16_t, float> predict(bc::Baritem* item)
	{
		return classic.findType(item);
		// return root.findType(item->getRootNode()->getChild(0));
	}


	Node root;

	ClassicTrainer classic;
	// Dataset dataset;
	Range width;
	Range height;
	PlusValues counters{};
	std::array<uint32_t, 3> truePositive{};
	std::array<uint32_t, 3> size{};

	std::array<uint32_t, 3> falsePositive{};
	std::array<uint32_t, 3> falseNegative{};
};

struct CocoImage
{
	BackImage img;
	int id;
};

std::vector<CocoImage> parseCocoImages(const BackJson& json, std::string prefixPath, const std::vector<std::string>& whitelist)
{
	std::vector<CocoImage> images;
	for (const auto& image : json["images"])
	{
		CocoImage img;
		auto fileName = image["file_name"].get<std::string>();
		img.id = image["id"].get<int>();

		bool good = false;
		for (const auto& search : whitelist)
		{
			if (fileName == search)
			{
				good = true;
				break;
			}
		}
		if (good)
		{
			img.img = imread(prefixPath + fileName);
			images.push_back(std::move(img));
		}
	}
	return images;
}

std::optional<uint32_t> checkAnnotationImage(const BackJson& annotate, const std::vector<CocoImage>& images)
{
	uint32_t cocoImageId = 0;
	auto imageId = annotate["image_id"].get<int>();

	for (const auto& image : images)
	{
		if (imageId == image.id)
		{
			return cocoImageId;
		}

		++cocoImageId;
	}

	return std::nullopt;
}


std::optional<Rect> parseBox(const BackJson& annotate)
{
	auto bbox = annotate["bbox"];
	if (bbox.size() != 4)
		return std::nullopt;

	int x = static_cast<int>(bbox[0]);
	int y = static_cast<int>(bbox[1]);
	int w = static_cast<int>(bbox[2]);
	int h = static_cast<int>(bbox[3]);

	Rect rect;
	rect.x.min = x;
	rect.x.max = x + w;
	rect.y.min = y;
	rect.y.max = y + h;

	// rect.x.min = x - 15;
	// rect.x.max = x + w + 15;
	// rect.y.min = y - 15;
	// rect.y.max = y + h + 15;
	return rect;
}

std::unique_ptr<bc::Baritem> makeBarcode(const BackImage& src, const bc::barstruct& constr, Rect rect)
{
	auto rectImg = src.getRectSafe(rect.x.min, rect.y.min, rect.x.max - rect.x.min, rect.y.max - rect.y.min);
	if (rectImg.width() == 0 || rectImg.height() == 0)
		return nullptr;

	auto item = bc::BarcodeCreator::create(rectImg, constr);
	// outData.dataset[ann["category_id"].get<int>()].root->add(item->getRootNode(), ann["category_id"].get<int>());
	assert(item->getRootNode()->getChildrenCount() == 1);

	return item;
}

std::unique_ptr<bc::Baritem> parseCocoAnnotationItem(const BackJson& ann, std::vector<CocoImage> images, const bc::barstruct& constr, Rect& outRect)
{
	auto imageId = checkAnnotationImage(ann, images);
	if (imageId == std::nullopt)
		return nullptr;

	auto box = parseBox(ann);
	if (box == std::nullopt)
		return nullptr;

	const BackImage& src = images[imageId.value()].img;
	outRect = box.value();

	return makeBarcode(src, constr, outRect);
}

void parseCocoAnnotationsTrain(DatasetData& outData, const bc::barstruct& constr, const std::string prefix = "train")
{
	std::string prefixPath(PATH.data(), PATH.size());
	prefixPath += prefix;
	prefixPath += "/";

	std::string jsonPath = prefixPath;
	jsonPath += "_annotations.coco.json";

	auto json = jsonFromFile(jsonPath);
	auto images = parseCocoImages(json, prefixPath, TRAIN);

	// Use nlohmann to parse JSON
	for (const auto& ann : json["annotations"])
	{
		Rect rect;
		auto item = parseCocoAnnotationItem(ann, images, constr, rect);
		if (!item)
			continue;

		auto categoryId = ann["category_id"].get<int>() - 1;

		int rscore = outData.addItem(item.get(), categoryId);
		++outData.counters[categoryId];

		[[maybe_unused]]
		auto [rtype, score] = outData.predict(item.get());
		assert(rtype == categoryId || score == 1);

		outData.width.min = std::min(outData.width.min, rect.x.min);
		outData.height.min = std::min(outData.height.min, rect.y.min);
		outData.width.max = std::max(outData.width.max, rect.x.max);
		outData.height.max = std::max(outData.height.max, rect.y.max);
	}

	std::cout << "Dataset trained:\n";
	for (size_t i = 0; i < 3; ++i)
	{
		std::cout << " Type " << i + 1 << ": Count=" << outData.counters[i] << std::endl;
	}
}

constexpr uint16_t NUM_CLASSES = 3;

double computeAccuracy(const std::array<std::array<int, 3>, 3>& cm)
{
    int correct = 0;
    int total = 0;

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
        {
            total += cm[i][j];
            if (i == j)
                correct += cm[i][j];
        }

    return total > 0 ? static_cast<double>(correct) / total : 0.0;
}

double computeBalancedAccuracy(const std::array<std::array<int, 3>, 3>& cm)
{
    double sumRecall = 0.0;

    for (int k = 0; k < 3; ++k)
    {
        int TP = cm[k][k];
        int FN = 0;

        for (int j = 0; j < 3; ++j)
            if (j != k)
                FN += cm[k][j];

        double recall = (TP + FN) > 0
            ? static_cast<double>(TP) / (TP + FN)
            : 0.0;

        sumRecall += recall;
    }

    return sumRecall / 3;
}

double computeMacroF1(
    const std::array<std::array<int, NUM_CLASSES>, NUM_CLASSES>& cm)
{
    double sumF1 = 0.0;

    for (int k = 0; k < NUM_CLASSES; ++k)
    {
        int TP = cm[k][k];
        int FP = 0;
        int FN = 0;

        for (int i = 0; i < NUM_CLASSES; ++i)
        {
            if (i != k)
            {
                FP += cm[i][k];
                FN += cm[k][i];
            }
        }

        double precision = (TP + FP) > 0
            ? static_cast<double>(TP) / (TP + FP)
            : 0.0;

        double recall = (TP + FN) > 0
            ? static_cast<double>(TP) / (TP + FN)
            : 0.0;

        double f1 = (precision + recall) > 0
            ? 2.0 * precision * recall / (precision + recall)
            : 0.0;

        sumF1 += f1;
    }

    return sumF1 / NUM_CLASSES;
}

void parseCocoAnnotationsTest(DatasetData& ds, const bc::barstruct& constr, const std::string prefix = "test")
{
	std::string prefixPath(PATH.data(), PATH.size());
	prefixPath += prefix;
	prefixPath += "/";

	std::string jsonPath = prefixPath;
	jsonPath += "_annotations.coco.json";

	auto json = jsonFromFile(jsonPath);

	auto images = parseCocoImages(json, prefixPath, TEST);

	auto outImages = images;

	std::array<std::array<int, 3>, 3> confusion = {};
	// Use nlohmann to parse JSON
	for (const auto& ann : json["annotations"])
	{
		Rect rect;
		auto item = parseCocoAnnotationItem(ann, images, constr, rect);
		if (!item)
			continue;

		const auto categoryId = ann["category_id"].get<int>() - 1;

		auto [rtype, score] = ds.predict(item.get());
		ds.size[categoryId]++;

		confusion[categoryId][rtype]++;

		Barscalar color;
		if (rtype == categoryId)
		{
			color = Barscalar(0, 255, 0);
			ds.truePositive[categoryId]++;
		}
		else
		{
			color = Barscalar(255, 0, 0);
			ds.falsePositive[rtype]++;
			ds.falseNegative[categoryId]++;
		}

		auto imageId = checkAnnotationImage(ann, images);
		auto& out = outImages[imageId.value()].img;

		drawCountur(out, rect, color, 2);
		drawPercentText(out, (score * 100), color, rect.x.min, rect.y.min - 10, 2);
	}

	std::cout << "Dataset evaluation:\n";
	for (size_t i = 0; i < 3; ++i)
	{
		uint32_t tp = ds.truePositive[i];
		float precision = (tp == 0) ? 0.0f : (float(tp) / float(ds.size[i]));
		std::cout << " Type " << i + 1 << ": Precision=" << precision << std::endl;
	}

	double acc = computeAccuracy(confusion);
	double balAcc = computeBalancedAccuracy(confusion);
	double macroF1 = computeMacroF1(confusion);

	std::cout << "Accuracy: " << acc << "\n";
	std::cout << "Balanced accuracy: " << balAcc << "\n";
	std::cout << "Macro-F1: " << macroF1 << "\n";

	for (const auto& outImg : outImages)
	{
		std::string outPath = "";
		// outPath += "/output/";
		// outPath += prefix;
		// outPath += "/";
		outPath += std::to_string(outImg.id);
		outPath += ".png";
		imwrite(outPath, outImg.img);
	}
}


void parseCocoAnnotationsTestAll(DatasetData& ds, const bc::barstruct& constr, const std::string prefix = "test")
{
	std::string prefixPath(PATH.data(), PATH.size());
	prefixPath += prefix;
	prefixPath += "/";

	std::string jsonPath = prefixPath;
	jsonPath += "_annotations.coco.json";

	auto json = jsonFromFile(jsonPath);


	std::unordered_map<int, BackImage> imagesCache;

	uint32_t count = 0;
	std::array<std::array<int, 3>, 3> confusion = {};
	auto start = std::chrono::high_resolution_clock::now();
	// Use nlohmann to parse JSON


	for (const auto& ann : json["annotations"])
	{
		auto imageId = ann["image_id"].get<int>();

		if (imagesCache.find(imageId) == imagesCache.end())
		{
			// Load image
			auto it = std::find_if(json["images"].begin(), json["images"].end(),
				[imageId](const BackJson& img) { return img["id"].get<int>() == imageId; });

			if (it == json["images"].end())
				continue;

			auto fileName = (*it)["file_name"].get<std::string>();
			BackImage img = imread(prefixPath + fileName, true);

			if (imagesCache.size() > 100)
			{
				imagesCache.clear();
			}

			std::cout << "Processing annotation for image ID " << imageId << "\n";
			imagesCache[imageId] = img;
		}


		auto rect = parseBox(ann);
		if (rect == std::nullopt)
			continue;

		auto item = makeBarcode(imagesCache[imageId], constr, rect.value());
		if (!item)
			continue;

		const auto categoryId = ann["category_id"].get<int>() - 1;

		auto [rtype, score] = ds.predict(item.get());
		ds.size[categoryId]++;

		confusion[categoryId][rtype]++;

		if (rtype == categoryId)
		{
			ds.truePositive[categoryId]++;
		}
	}
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = end - start;
	std::cout << "Function duration: " << duration.count() << " seconds" << std::endl;

	std::cout << "Dataset evaluation:\n";
	for (size_t i = 0; i < 3; ++i)
	{
		uint32_t tp = ds.truePositive[i];
		float precision = (tp == 0) ? 0.0f : (float(tp) / float(ds.size[i]));
		std::cout << " Type " << i + 1 << ": Precision=" << precision << std::endl;
	}

	double acc = computeAccuracy(confusion);
	double balAcc = computeBalancedAccuracy(confusion);
	double macroF1 = computeMacroF1(confusion);

	std::cout << "Accuracy: " << acc << "\n";
	std::cout << "Balanced accuracy: " << balAcc << "\n";
	std::cout << "Macro-F1: " << macroF1 << "\n";


}

BackImage findObjectsOnImage(const BackImage& src, const bc::barstruct& constr, const DatasetData& ds)
{
	BackImage out = src;
	std::unique_ptr<bc::Baritem> item = bc::BarcodeCreator::create(src, constr);

	std::array<Barscalar, 3> colors;
	colors[0] = Barscalar(0, 255, 0);
	colors[1] = Barscalar(0, 0, 255);
	colors[2] = Barscalar(255, 0, 0);

	for (size_t i = 0; i < item->barlines.size(); ++i)
	{
		auto line = item->barlines[i];

		const auto& matr = item->barlines[i]->matr;
		if (matr.size() < 100)
			continue;

		if (matr.size() > 100000)
			continue;

		auto [type, score] = ds.root.findType(line);

		float accuracy = score;
		// assert(accuracy >= 0.0f && accuracy <= 1.0f);
		if (accuracy < 0.1)
			continue;

		// score = 100 - std::min<uint8_t>(score, 100);

		Barscalar color = colors[type % colors.size()];

		// if (matr.size() < 1000)
		// 	continue;

		// if (matr.size() > 2000)
		// 	continue;

		auto rect = findCountur(matr);

		drawCountur(out, rect, color, 2);
		drawPercentText(out, (accuracy * 100), Barscalar(0, 0, 0), rect.x.min, rect.y.min - 10, 2);
	}

	return out;
}

void objectClassification(const bc::barstruct& constr)
{
	DatasetData ds;
	parseCocoAnnotationsTrain(ds, constr, "train");
	parseCocoAnnotationsTestAll(ds, constr, "test");
}


RetLayers findObjects(InOutLayer iol, const MLSettings& setting)
{
	RetLayers ret;
	// bc::barstruct constr = getConstr(setting);
	// findObjectsOnImage

	return ret;
}

static AlgFuncRegister registerFindObjects("find_objects", findObjects, mkSettingsType);

AutoRunRegister registerParseCocoAnnotations([]()
{
	std::string path(PATH.data(), PATH.size());
	path += "/train/";
	path += TRAIN[0];
	BackImage testImg = imread(path);
	// BackImage testImg = imread("/Users/sam/Edu/bar/12/10.png", true);

	bc::barstruct constr;
	constr.proctype = bc::ProcType::Radius;
	constr.createGraph = true;
	constr.createBinaryMasks = false;
	constr.attachMode = bc::AttachMode::morePointsEatLow;
	// objectClassification(constr);

	{
		DatasetData ds;
		parseCocoAnnotationsTrain(ds, constr, "train");
		parseCocoAnnotationsTest(ds, constr, "test");
	}
	exit(0);
});