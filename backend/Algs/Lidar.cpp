#include <vector>
#include <memory>
#include <random>


#include "../extra/BarcodeCreatorCloud.h"

#include "../MatrImg.h"
#include "../MLSettings.h"
#include "../Layers/layerInterface.h"
#include "../Layers/Rasterlayers.h"


#include "../backend/project.h"
// import MLSettings;
// import MatrModule;
//import IAlgorithm
#include "../backend/Interfaces/IAlgorithm.h"
//import AlgUtils;
//import SimpleBar;
#include "../backend/Algs/SimpleBar.h"

#include <fstream>
#include <iostream>
#include <string>
#include "../Layers/RasterLineLayer.h"
#include "../GeoProcessor.h"

using ull = unsigned long long;

struct LasOut
{
	// bc::CloudPointsBarcode::CloudPoints cloudPoints;
	std::unordered_map<ull, int> allPoints;
	ull x = 0;
	ull y = 0;
	ull width = 0;
	ull height = 0;
};
class LasParser
{
public:
	static LasOut parseLasFile(std::string_view filePath)
	{
		LasOut out;

		int startX = 999999, startY = 999999;
		int endX = 0, endY = 0;

		StateBinFile::BinStateReader stateReader;
		stateReader.open(filePath.data(), true);

		bool first = true;
		bool dummy = false;
		while (stateReader.pBool(dummy))
		{
			bc::CloudPointsBarcode::CloudPoint point(0, 0, 0);
			point.x = stateReader.pInt(point.x);
			point.y = stateReader.pInt(point.y);
			point.z = stateReader.pInt(point.z);

			if (first)
			{
				endX = startX = point.x;
				endY = startY = point.y;
				first = false;
			}

			if (point.x < startX)
				startX = point.x;
			if (point.y < startY)
				startY = point.y;
			if (point.x > endX)
				endX = point.x;
			if (point.y > endY)
				endY = point.y;

			out.allPoints.insert_or_assign(static_cast<ull>(point.x) << 32 | static_cast<ull>(point.y), point.z);
		}

		out.x = startX;
		out.y = startY;
		out.width = endX - startX;
		out.height = endY - startY;
		std::cout << "Parsed " << out.allPoints.size() << " points from LAS file." << std::endl;
		std::cout << "X range: " << startX << " to " << endX << std::endl;
		std::cout << "Y range: " << startY << " to " << endY << std::endl;
		std::cout << "Width: " << out.width << ", Height: " << out.height << std::endl;

		return out;
	}
};
class PixelIterator
{
public:
	PixelIterator(ull start, ull end, ull iterator = 1)
		: start(start), end(end), iterator(iterator)
	{ }

	bool notEnded() const
	{
		return start < end;
	}

	void iterate()
	{
		start += iterator;
	}

	ull value() const { return start; }
private:
	ull start;
	ull end;
	ull iterator = 10;
};


const ull chunkSize = 1500;
const ull chunkOversize = 100;
const ull step = 10;

int getTileSize(int x, int step)
{
	const int a = x / step;
	const int b = x % step;

	return a + (b != 0 ? 1 : 0);
}

using CallbackFunc = std::function<bool(bc::Baritem&, const TileProvider iolProv)>;
void processLas(const LasOut& outlas, bc::barstruct bc, float factor, bool fromZero, CallbackFunc callback)
{
	const ull fullChunkSize = chunkSize + chunkOversize;

	const ull chunkWidthToRead = fullChunkSize * step;
	const ull chunkRealToRead = chunkSize * step;

	const ull lasEndY = outlas.y + outlas.height;

	// Read the real size
	for (ull chunkStartX = outlas.x; chunkStartX < outlas.x + outlas.width; chunkStartX += chunkRealToRead)
	{
		for (ull chunkStartY = outlas.y; chunkStartY < lasEndY; chunkStartY += chunkRealToRead)
		{
			// Chunk read pos
			const ull chunkEndX = std::min(chunkStartX + chunkWidthToRead, outlas.x + outlas.width);
			const ull chunkEndY = std::min(chunkStartY + chunkWidthToRead, lasEndY);

			// Chunk in output size
			BackImage chunk(getTileSize((chunkEndX - chunkStartX), step), getTileSize((chunkEndY - chunkStartY), step), BarType::FLOAT32_1);


			// read the chunk in real size
			int outX = 0;
			for (PixelIterator iterX(chunkStartX, chunkEndX, step); iterX.notEnded();iterX.iterate())
			{
				int outY = fromZero ? chunk.height() : 0;
				for (PixelIterator iterY(chunkStartY, chunkEndY, step); iterY.notEnded(); iterY.iterate())
				{
					// Read the rect and Aproximate by step
					float sum = 0;
					float count = 0;
					for (PixelIterator aproximateX(iterX.value(), iterX.value() + step); aproximateX.notEnded(); aproximateX.iterate())
					{
						for (PixelIterator aproximateY(iterY.value(), iterY.value() + step); aproximateY.notEnded(); aproximateY.iterate())
						{
							bc::CloudPointsBarcode::CloudPoint point(aproximateX.value(), aproximateY.value(), 0);
							auto it = outlas.allPoints.find(static_cast<ull>(point.x) << 32 | static_cast<ull>(point.y));
							if (it != outlas.allPoints.end())
							{
								// std::cout << "Found [" << aproximateX.value() << "," <<aproximateY.value() << "] = " << it->second << std::endl;
								sum += it->second;
								++count;
							}
						}
					}
					// Invert the Y
					chunk.set(outX, fromZero ? --outY : outY++, count == 0 ? -9999 : (sum / count));
				}

				++outX;
			}


			// Invert the Y
			const ull yCurStart = fromZero ? (lasEndY - chunkEndY) : (chunkStartY - outlas.y);
			const TileProvider localProjection(factor, (chunkStartX - outlas.x) / step, yCurStart / step);

			const TileProvider globalProjection(factor, chunkStartX, chunkStartY);

			std::cout << "------Added chunk-----\n";
			std::cout << "X: " << localProjection.offset.x << " Y:" << localProjection.offset.y << std::endl;

			bc::BarcodeCreator bcc;
			std::unique_ptr<bc::Baritem> item = bc::BarcodeCreator::create(chunk, bc);

			bool ret = callback(*item.get(), fromZero ? localProjection : globalProjection);
			if (!ret)
			{
				return;
			}

			std::cout << "------Move to the next chunk-----\n";

		}
	}
}
RetLayers exeLidar(InOutLayer iol, const MLSettings& setting)
{
	std::string_view lasFilePath = "/Users/sam/H/Programs/imgui/SatHomology/las1.raw";
	LasOut outlas = LasParser::parseLasFile(lasFilePath);
	const ull lasEndY = outlas.y + outlas.height;


	bc::barstruct bc = getConstr(setting);
	bc.coltype = bc::ColorType::native;
	bc.createBinaryMasks = true;
	bc.createGraph = true;

	// for (const auto& point : outlas.points)
	// {
	// 	cloudPoints.emplace_back(point.x - outlas.x, point.y - outlas.y, point.z);
	// }
	Project* proj = Project::getProject();

	IRasterLayer* input = proj->getInRaster(iol);

	const float factor = (outlas.height / step) / input->displayHeight();

	RasterLineLayer* layer = proj->addLayerData<RasterLineLayer>();
	layer->init(input, proj->getMeta());
	layer->prov.displayFactor = factor;

	processLas(outlas, bc, factor, true, [&](bc::Baritem& item, const TileProvider iolProv) {

		CachedBaritemHolder holder;
		holder.init(&item, nullptr);

		layer->addHolder(holder, iolProv, nullptr);

		return true;
	});

	return {layer};
}

void processAll()
{
	std::array<std::string_view, 3> files = {
		"/Users/sam/H/Programs/imgui/SatHomology/lidara.raw",
		"/Users/sam/H/Programs/imgui/SatHomology/las1.raw",
		"/Users/sam/H/Programs/imgui/SatHomology/las2.raw"
	};

	std::array<bc::ProcType, 3> procs = {
		bc::ProcType::f0t255,
		bc::ProcType::f255t0,
		bc::ProcType::Radius
	};

	std::array<std::string_view, 9> labels = {
		"Ground_2020 f0t255",
		"Ground_2020 f255t0",
		"Ground_2020 Radius",
		"2020_1 ch1 f0t255",
		"2020_1 ch1 f255t0",
		"2020_1 ch1 Radius",
		"2020_1 ch2 f0t255",
		"2020_1 ch2 f255t0",
		"2020_1 ch2 Radius"
	};
	static_assert(files.size() == procs.size());
	static_assert(files.size() * procs.size() == labels.size());

	int labelCounter = 0;
	for (size_t i = 0; i < files.size(); ++i)
	{
		std::cout << "Processing file: " << files[i] << std::endl;

		std::string_view lasFilePath = files[i];
		LasOut outlas = LasParser::parseLasFile(lasFilePath);

		bc::barstruct bcs;
		bcs.coltype = bc::ColorType::native;
		bcs.createBinaryMasks = true;
		bcs.createGraph = true;

		for (size_t j = 0; j < procs.size(); ++j)
		{
			std::cout << "Step: " << j << std::endl;
			bcs.proctype = procs[j];

			const std::string_view label = labels[labelCounter++];
			ShapeFile shp(label);


			processLas(outlas, bcs, 100, false, [&](bc::Baritem& item, const TileProvider iolProv)
			{
				for (size_t i = 0; i < item.barlines.size(); ++i)
				{
					const bc::barline& curLine = *item.barlines[i];
					shp.writePolygonRecord(curLine, iolProv);
				}
				return true;
			});
			shp.close();
			std::cout << "Write to " << label << std::endl;
			return;
		}
	}
}



void test()
{
	std::string_view lasFilePath = "/Users/sam/H/Programs/imgui/SatHomology/lidara.raw";
	LasOut outlas = LasParser::parseLasFile(lasFilePath);

	bc::barstruct bc;
	bc.coltype = bc::ColorType::native;
	bc.createBinaryMasks = true;
	bc.createGraph = true;


	int a = 1;
	ShapeFile shp("test");
	processLas(outlas, bc, 100, false, [&](bc::Baritem& item, const TileProvider iolProv)
	{
		for (size_t i = 0; i < item.barlines.size(); ++i)
		{
			const bc::barline& curLine = *item.barlines[i];
			shp.writePolygonRecord(curLine, iolProv, step);
		}
		return ++a < 1;
	});

	shp.close();
	exit(0);
}


void rtest1()
{
	bc::barvector inPoints;

	inPoints.push_back({0,0, 0.f});
	inPoints.push_back({0,1, 0.f});
	inPoints.push_back({0,2, 0.f});
	mcountor out;
	getCountourOder(inPoints, out, true);

	assert(out.size() == 4);

	assert(out[0].x == 1);
	assert(out[0].y == 0);

	assert(out[1].x == 1);
	assert(out[1].y == 3);

	assert(out[2].x == 0);
	assert(out[2].y == 3);

	assert(out[3].x == 0);
	assert(out[3].y == 0);
}


void rtest2()
{
	bc::barvector inPoints;

	inPoints.push_back({2,1, 0.f});
	inPoints.push_back({1,2, 0.f});
	mcountor out;
	getCountourOder(inPoints, out, true);

	assert(out.size() == 4);

	assert(out[0] == BackPixelPoint(1,2));

	assert(out[1].x == 1);
	assert(out[1].y == 0);

	assert(out[2].x == 1);
	assert(out[2].y == 3);

	assert(out[3].x == 0);
	assert(out[3].y == 3);
}

void rtest3()
{
	bc::barvector inPoints;
	inPoints.push_back({1,1,  0.f});
	// inPoints.push_back({2,1,  0.f});

	mcountor out;
	getCountourOder(inPoints, out, true);

	assert(out.size() == 4);

	assert(out[0] == BackPixelPoint(1,2));

	assert(out[1].x == 1);
	assert(out[1].y == 0);

	assert(out[2].x == 1);
	assert(out[2].y == 3);

	assert(out[3].x == 0);
	assert(out[3].y == 3);
}

void tests()
{
	// rtest1();
	// rtest2();
	rtest3();
}
static AlgFuncRegister registerLidar("Lidar", exeLidar, mkSettingsType);
class AutoRun
{
public:
	AutoRun()
	{
		// exeLidar({}, mkSettingsType());
		// processAll();
		test();
		// tests();
	}
};
static AutoRun autoee;
