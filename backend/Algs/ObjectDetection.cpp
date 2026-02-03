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
void drawPercentText(BackImage& out, int percent, const Barscalar& color, int x = 0, int y = 0, int scale = 1)
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
	"4f833867-273e-4d73-8bc3-cb2d9ceb54ef_1060_1060_jpg.rf.8e289124678d6ae3dee96979050174c0.jpg",
	"4fdabd34-a2fd-4f0a-bb48-01fe043f1499_0_1060_jpg.rf.067f2467b08ae9a2706e6df2c234e295.jpg",
	"38ff8d64-3460-4f83-bf9c-383832aeba0d_0_1060_jpg.rf.cc6f9fbb05f103d7ce0ebfecae8776a8.jpg",
	"P2672__1-0__3000___600_png_jpg.rf.596ca436c207483c9e8afa3e2710fb94.jpg",
	"P2753__1-0__0___1200_png_jpg.rf.a7109280c463052b2e9672296282f090.jpg",
	"P2754__1-0__1800___1200_png_jpg.rf.b7a1685eeacf8bac3949219f434f836d.jpg"
};

const std::vector<std::string> TEST
{
	"0fec7f7ff_png_jpg.rf.c7512d2139a78b930e5adc81a1b1f250.jpg",
	"2cdbcdd8e_png_jpg.rf.b542fbf7049fbb307b7f78a8e29aad6a.jpg",
	"3e321b8a-9504-45aa-82b4-16158e28e290_1060_1060_jpg.rf.362d727519b454eeac2b42fb35142d3e.jpg",
	"03f84930-e2be-4c19-9afc-0dc26d02538e_1060_0_jpg.rf.c8ac4f9068dde47da4ac0211f59e751b.jpg",
	"15691_png_jpg.rf.fa449ee7d1d79b9307e7c7effb376344.jpg",
	"P1412__1-0__3000___0_png_jpg.rf.50630bd0bc29553f3c61b2b6a9c5ab48.jpg"
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
			result.hits[i] -= distance;
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

struct DatasetEntry
{
	// uint16_t typeId;
	Node root;
	PlusValues counters{};

	// std::vector<int> itemIds;

	// void buildDecisionTree()
	// {
	// 	itemIds.clear();
	// 	for (size_t i = 0; i < items.size(); ++i)
	// 	{
	// 		auto& item = items[i];
	// 		int minX = std::numeric_limits<int>::max();
	// 		int minY = std::numeric_limits<int>::max();
	// 		int maxX = std::numeric_limits<int>::min();
	// 		int maxY = std::numeric_limits<int>::min();
	// 		for (const auto& barline : item->barlines)
	// 		{
	// 			for (const auto& pm : barline->matr)
	// 			{
	// 				minX = std::min(minX, pm.getX());
	// 				minY = std::min(minY, pm.getY());
	// 				maxX = std::max(maxX, pm.getX());
	// 				maxY = std::max(maxY, pm.getY());
	// 			}
	// 		}
	// 		float centroidX = (minX + maxX) / 2.0f;
	// 		float centroidY = (minY + maxY) / 2.0f;
	// 		float width = maxX - minX;
	// 		float height = maxY - minY;
	// 		Trainer<4>::Hash h;
	// 		h.key[0] = centroidX;
	// 		h.key[1] = centroidY;
	// 		h.key[2] = width;
	// 		h.key[3] = height;
	// 		h.value = 0.0f; // not used
	// 		index.add(h);
	// 		itemIds.push_back(i);
	// 	}
	// 	index.train();
	// }

	// int findClosest(const bc::Baritem& query)
	// {
	// 	int minX = std::numeric_limits<int>::max();
	// 	int minY = std::numeric_limits<int>::max();
	// 	int maxX = std::numeric_limits<int>::min();
	// 	int maxY = std::numeric_limits<int>::min();
	// 	for (const auto& barline : query.barlines)
	// 	{
	// 		for (const auto& pm : barline->matr)
	// 		{
	// 			minX = std::min(minX, pm.getX());
	// 			minY = std::min(minY, pm.getY());
	// 			maxX = std::max(maxX, pm.getX());
	// 			maxY = std::max(maxY, pm.getY());
	// 		}
	// 	}
	// 	float centroidX = (minX + maxX) / 2.0f;
	// 	float centroidY = (minY + maxY) / 2.0f;
	// 	float width = maxX - minX;
	// 	float height = maxY - minY;
	// 	Trainer<4>::Hash h;
	// 	h.key[0] = centroidX;
	// 	h.key[1] = centroidY;
	// 	h.key[2] = width;
	// 	h.key[3] = height;
	// 	int lineId = index.getCloser(h);
	// 	return itemIds[lineId];
	// }
};

using Dataset = std::array<DatasetEntry, 3>;

struct DatasetData
{
	Node root;
	// Dataset dataset;
	Range width;
	Range height;
	PlusValues counters{};
	std::array<uint32_t, 3> truePositive{};
	std::array<uint32_t, 3> falsePositive{};
	std::array<uint32_t, 3> size{};
};


void parseCocoAnnotations(DatasetData& outData, const bc::barstruct& constr, std::string prefix = "test", bool trainDataset = false)
{
	std::string prefixPath(PATH.data(), PATH.size());
	prefixPath += prefix;
	prefixPath += "/";

	std::string jsonPath = prefixPath;
	jsonPath += "_annotations.coco.json";

	auto json = jsonFromFile(jsonPath);

	std::vector<int> goodImageIds;
	std::vector<BackImage> goodImages;
	for (const auto& image : json["images"])
	{
		auto name = image["file_name"];
		auto id = image["id"];

		bool good = false;
		for (const auto& search : TRAIN)
		{
			if (name.get<std::string>() == search)
			{
				goodImageIds.push_back(id.get<int>());

				goodImages.push_back(imread(prefixPath + search));
				break;
			}
		}
	}

	// Use nlohmann to parse JSON
	for (const auto& ann : json["annotations"])
	{
		bool skip = true;
		int id = 0;
		for (const auto& gid : goodImageIds)
		{
			if (ann["image_id"].get<int>() == gid)
			{
				skip = false;
				break;
			}

			++id;
		}

		if (skip)
			continue;

		auto bbox = ann["bbox"];
		if (bbox.size() != 4) continue;
		int x = static_cast<int>(bbox[0]);
		int y = static_cast<int>(bbox[1]);
		int w = static_cast<int>(bbox[2]);
		int h = static_cast<int>(bbox[3]);
		Rect rect;
		rect.x.min = x;
		rect.x.max = x + w;
		rect.y.min = y;
		rect.y.max = y + h;

		BackImage& src = goodImages[id];

		auto rectImg = src.getRect(rect.x.min, rect.y.min, rect.x.max - rect.x.min, rect.y.max - rect.y.min);

		auto item = bc::BarcodeCreator::create(rectImg, constr);
		// outData.dataset[ann["category_id"].get<int>()].root->add(item->getRootNode(), ann["category_id"].get<int>());
		assert(item->getRootNode()->getChildrenCount() == 1);
		auto categoryId = ann["category_id"].get<int>() - 1;

		if (id == 2)
			id = 2;

		if (trainDataset)
		{
			int rscore = outData.root.add(item->getRootNode(), categoryId);
			outData.counters[categoryId] += rscore;

			auto [rtype, score] = outData.root.findType(item->getRootNode()->getChild(0));
			assert(rtype == categoryId || score == 1);

			outData.width.min = std::min(outData.width.min, rect.x.min);
			outData.height.min = std::min(outData.height.min, rect.y.min);
			outData.width.max = std::max(outData.width.max, rect.x.max);
			outData.height.max = std::max(outData.height.max, rect.y.max);
			outData.size[categoryId]++;
		}
		else
		{
			auto [rtype, score] = outData.root.findType(item->getRootNode()->getChild(0));
			if (rtype == categoryId)
			{
				outData.truePositive[categoryId]++;
			}
			else
			{
				outData.falsePositive[rtype]++;
			}
			// Process test data if needed
		}
	}
}


BackImage findObjects(const BackImage& src, const bc::barstruct& constr)
{
	BackImage out = src;

	DatasetData ds;
	parseCocoAnnotations(ds, constr, "train");

	// std::unique_ptr<bc::Baritem> containner(bcc.run(&src, constr, dummy));
	// bc::Baritem* item = containner.get();

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


RetLayers findObjects(InOutLayer iol, const MLSettings& setting)
{
	RetLayers ret;
	bc::barstruct constr = getConstr(setting);

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
	constr.proctype = bc::ProcType::f255t0;
	constr.createGraph = true;
	constr.createBinaryMasks = false;
	constr.attachMode = bc::AttachMode::firstEatSecond;
	BackImage out = findObjects(testImg, constr);
	imwrite("/Users/sam/H/Programs/imgui/SatHomology/out.png", out);
});