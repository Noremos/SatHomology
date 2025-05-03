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

struct LasPoint {
	float x, y, z;
	unsigned short intensity;
};

struct LasOut
{
	// bc::CloudPointsBarcode::CloudPoints cloudPoints;
	std::unordered_map<unsigned long long, int> allPoints;
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
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

RetLayers exeLidar(InOutLayer iol, const MLSettings& setting)
{
	std::string_view lasFilePath = "/Users/sam/H/Programs/imgui/SatHomology/lidara.raw";
	LasOut outlas = LasParser::parseLasFile(lasFilePath);

	const int chunkSize = 500;


	// for (const auto& point : outlas.points)
	// {
	// 	cloudPoints.emplace_back(point.x - outlas.x, point.y - outlas.y, point.z);
	// }
	Project* proj = Project::getProject();

	IRasterLayer* input = proj->getInRaster(iol);
	LayerProvider prov;
	prov.init(outlas.width, outlas.height, input->displayWidth(), chunkSize);

	RasterLineLayer* layer = proj->addLayerData<RasterLineLayer>();
	layer->init(input, proj->getMeta());

	const int chunkWid = outlas.width / chunkSize;
	const int chunkHei = outlas.height / chunkSize;

	BackImage mask(chunkSize, chunkSize, BarType::BYTE8_1);
	mask.fill((uchar)1);

	for (int cw = 0; cw < chunkWid; cw++)
	{
		for (int ch = 0; ch < chunkHei; ch++)
		{
			BackImage chunk(chunkSize, chunkSize, BarType::FLOAT32_1);

			int startX = outlas.x + cw * chunkSize;
			int startY = outlas.y + ch * chunkSize;
			int endX = std::min(startX + chunkSize, outlas.x + outlas.width);
			int endY = std::min(startY + chunkSize, outlas.y + outlas.height);

			int found = 0;
			for (int x = startX; x < endX; x++)
			{
				for (int y = startY; y < endY; y++)
				{
					bc::CloudPointsBarcode::CloudPoint point(x, y, 0);
					auto it = outlas.allPoints.find(static_cast<unsigned long long>(point.x) << 32 | static_cast<unsigned long long>(point.y));
					if (it != outlas.allPoints.end())
					{
						std::cout << "Found [" << x - startX << "," << y - startY << "] = " << it->second << std::endl;
						chunk.set(x - startX, y - startY, (float)it->second);
						++found;
					}
					else
						mask.set(x - startX, y - startY, (uchar)0);
				}
			}

			std::cout << "Found " << found << " pixels";

			bc::barstruct bc;
			bc.proctype = bc::ProcType::Radius;
			bc.coltype = bc::ColorType::native;
			bc.mask = &mask;
			bc.maskId = 1;

			bc::BarcodeCreator bcc;
			std::unique_ptr<bc::Baritem> item = bc::BarcodeCreator::create(chunk, bc);

			CachedBaritemHolder holder;
			holder.init(item.get(), nullptr);

			TileProvider iolProv(1.0, startX - outlas.x, startY - outlas.y);
			layer->addHolder(holder, iolProv, nullptr);

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
