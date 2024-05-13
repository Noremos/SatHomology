module;
#include <memory>
#include <thread>

export module ProjectSettings;

import MetadataCoreIO;
import RefSettings;

const char* const jsn_displayFacto = "step";
const char* const jsn_imgMinVal = "imgMinVal";
const char* const jsn_imgMaxVal = "imgMaxVal";
const char* const jsn_geojsonPath = "geojsonPath";
const char* const jsn_imgPath = "imgPath";
const char* const jsn_classfiles = "barfiles";
const char* const jsn_dispalyImg = "subImageIndex";
const char* const jsn_alg = "algIndex";
const char* const jsn_tileSize = "tileSize";
const char* const jsn_tileOffset = "tileOffset";

export struct ProjectSettings
{
	int metaCounter = 0;
	int threadsCount;
	bool runAsync = false;
	int layerCounter = 0;
	std::unique_ptr<MetadataProvider> metaprov;
	ProjectSettings()
	{
		threadsCount = std::thread::hardware_concurrency();
	}

	RefSettings& getIO()
	{
		return settings;
	}

	MetadataProvider& getMeta()
	{
		return *metaprov.get();
	}

	static ProjectSettings globalSettings;
private:
	RefSettings settings =
	{

		//{jsn_displayFacto, u_displayFactor},
		//{jsn_imgMaxVal, u_imgMaxVal},
		//{jsn_imgMinVal, u_imgMinVal},
		{"metacounter", metaCounter},
		{"threadsCount", threadsCount},
		{"runAsync", runAsync},
		{"layerCounter", layerCounter}


		//{jsn_imgPath, u_imgPath},
		//{jsn_geojsonPath, this->u_geojsonPath},
		//{jsn_classfiles, this->u_classCache},
		//{jsn_dispalyImg, this->u_subImageIndex},
		//{jsn_alg, this->u_algorithm},
		//{jsn_tileSize, this->tileSize},
		//{jsn_tileOffset, this->tileOffset}
	};
};

ProjectSettings ProjectSettings::globalSettings;

export ProjectSettings& getSettings()
{
	return ProjectSettings::globalSettings;
}
