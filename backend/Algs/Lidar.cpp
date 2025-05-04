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

struct LasPoint {
	float x, y, z;
	unsigned short intensity;
};

struct LasOut
{
	// bc::CloudPointsBarcode::CloudPoints cloudPoints;
	std::unordered_map<unsigned long long, int> allPoints;
	unsigned long long x = 0;
	unsigned long long y = 0;
	unsigned long long width = 0;
	unsigned long long height = 0;
};

struct Header
{
	char fileSignature[4]; // "LASF"
	unsigned short fileSourceId;
	unsigned short globalEncoding;
	unsigned int projectId1;
	unsigned short projectId2;
	unsigned short projectId3;
	char versionMajor;
	char versionMinor;
	char systemIdentifier[32];
	char generatingSoftware[32];
	unsigned int fileCreationDayOfYear;
	unsigned int fileCreationYear;
	unsigned int headerSize; // 227 bytes
	unsigned int offsetToPointData;
};
struct PointDataRecordFormat
{
	unsigned char dataRecordFormat;
	unsigned short pointDataRecordLength;
	unsigned int numberOfPointRecords;
	unsigned int numberOfPointsByReturn[5];
	unsigned int xScaleFactor;
	unsigned int yScaleFactor;
	unsigned int zScaleFactor;
	int xOffset;
	int yOffset;
	int zOffset;
};
struct PointDataRecord
{
	unsigned int x;
	unsigned int y;
	unsigned int z;
	unsigned short intensity;
	unsigned char returnNumber;
	unsigned char numberOfReturns;
	unsigned char classification;
	unsigned char scanDirectionFlag;
	unsigned char edgeOfFlightLine;
	unsigned char classificationFlags;
	unsigned char userData;
	unsigned short pointSourceId;
};
struct PointData
{
	PointDataRecordFormat format;
	std::vector<PointDataRecord> points;
};
struct PointDataRecordVLR
{
	unsigned short recordId;
	unsigned short recordLengthAfterHeader;
	unsigned short description;
	unsigned int userId;
	unsigned int recordNumber;
	unsigned int recordOffset;
};
struct PointDataRecordVLRs
{
	unsigned short recordCount;
	std::vector<PointDataRecordVLR> records;
};
struct PointDataRecordVLRHeader
{
	unsigned short recordId;
	unsigned short recordLengthAfterHeader;
	unsigned short description;
	unsigned int userId;
	unsigned int recordNumber;
	unsigned int recordOffset;
};
struct PointDataRecordVLRsHeader
{
	unsigned short recordCount;
	std::vector<PointDataRecordVLRHeader> records;
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

			out.allPoints.insert_or_assign(static_cast<unsigned long long>(point.x) << 32 | static_cast<unsigned long long>(point.y), point.z);
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
using ull = unsigned long long;
class PixelIterator
{
public:
	PixelIterator(unsigned long long start, unsigned long long end, ull iterator = 1)
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

RetLayers exeLidar(InOutLayer iol, const MLSettings& setting)
{
	std::string_view lasFilePath = "/Users/sam/H/Programs/imgui/SatHomology/las1.raw";
	LasOut outlas = LasParser::parseLasFile(lasFilePath);

	const unsigned long long chunkSize = 100;
	const unsigned long long chunkOversize = 0;
	const unsigned long long step = 10;


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

	const unsigned long long fullChunkSize = chunkSize + chunkOversize;

	const unsigned long long chunkWidthToRead = fullChunkSize * step;
	const unsigned long long chunkRealToRead = chunkSize * step;

	const unsigned long long lasEndY = outlas.y + outlas.height;

	// Read the real size
	for (unsigned long long chunkStartX = outlas.x; chunkStartX < outlas.x + outlas.width; chunkStartX += chunkRealToRead)
	{
		for (unsigned long long chunkStartY = outlas.y; chunkStartY < lasEndY; chunkStartY += chunkRealToRead)
		{
			// Chunk read pos
			const unsigned long long chunkEndX = std::min(chunkStartX + chunkWidthToRead, outlas.x + outlas.width);
			const unsigned long long chunkEndY = std::min(chunkStartY + chunkWidthToRead, lasEndY);

			// Chunk in output size
			BackImage chunk((chunkEndX - chunkStartX) / step, (chunkEndY - chunkStartY) / step, BarType::FLOAT32_1);


			// read the chunk in real size
			int outX = 0;
			for (PixelIterator iterX(chunkStartX, chunkEndX, step); iterX.notEnded();iterX.iterate())
			{
				int outY = chunk.height();
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
							auto it = outlas.allPoints.find(static_cast<unsigned long long>(point.x) << 32 | static_cast<unsigned long long>(point.y));
							if (it != outlas.allPoints.end())
							{
								// std::cout << "Found [" << aproximateX.value() << "," <<aproximateY.value() << "] = " << it->second << std::endl;
								sum += it->second;
								++count;
							}
						}
					}
					// Invert the Y
					chunk.set(outX, --outY, count == 0 ? -9999 : (sum / count));
				}

				++outX;
			}

			std::cout << "Found " << found << " pixels";

			bc::barstruct bc;
			bc.proctype = bc::ProcType::Radius;
			bc.coltype = bc::ColorType::native;

			bc::BarcodeCreator bcc;
			std::unique_ptr<bc::Baritem> item = bc::BarcodeCreator::create(chunk, bc);

			{
				ShapeFile shp("test");
				for (size_t i = 0; i < item->barlines.size(); ++i)
				{
					const bc::barline& curLine = *item->barlines[i];
					shp.writePolygonRecord(curLine);
				}

				shp.close();
			}

			CachedBaritemHolder holder;
			holder.init(item.get(), nullptr);

			// Invert the Y
			const ull yCurStart = lasEndY - chunkEndY;

			const TileProvider iolProv(factor, (chunkStartX - outlas.x) / step, yCurStart / step);
			layer->addHolder(holder, iolProv, nullptr);

			std::cout << "------Added-----\n";
			std::cout << "X: " << iolProv.offset.x << " Y:" << iolProv.offset.y << std::endl;



			return {layer};
		}
	}
	return {layer};
}


static AlgFuncRegister registerLidar("Lidar", exeLidar, mkSettingsType);
class AutoRun
{
public:
	AutoRun()
	{
		exeLidar({}, mkSettingsType());
	}
};
// static AutoRun autoee;
