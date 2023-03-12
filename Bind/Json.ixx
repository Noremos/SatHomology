 module;
 #include "Common.h"
 #include <iostream>

#ifdef _DEBUG #pragma comment(lib, "json/jsoncpp_debug.lib") #else #pragma comment(lib, "json/jsoncpp.lib")
#endif

#include "jsoncpp/json.h"

 export module JsonCore;

 //export import <json.hpp>;

 export using BackJson = Json::Value;
 export using JsonObject = BackJson;
 export using JsonArray = BackJson;


 export BackJson jsonFromFile(const BackPathStr& path)
 {
	BackJson temp;
 	BackFileReader file;
 	file.open(path);
 	if (!file.is_open())
 	{
 		std::cerr << "Couldn't open a file.";
 		return BackJson();
 	}
 	file >> temp;
 	file.close();

 	return temp;
 }

 export void jsonToFile(const BackJson& json, const BackPathStr& path)
 {
 	BackFileWriter file;
 	file.open(path, std::ios::trunc);
 	if (!file.is_open())
 	{
 		std::cerr << "Couldn't open a file.";
 		return;
 	}

    Json::StreamWriterBuilder wbuilder;
    // Configure the Builder, then ...
    std::string outputConfig = Json::writeString(wbuilder, json);
 	file << outputConfig;
 	file.close();
 }
