module;

#include "../Bind/Json.h"

// interface_part.cppm
export module JsonState;
export import :interface_part;

// import JsonCore;
import BackBind;



// JsonArrayIOState* JsonObjectIOStateReader::arrayBegin(const BackString& name, int& size)
// {
// 	const BackJson& jarr = json[name];
// 	size = jarr.size();
// 	curArray = std::make_unique<JsonArrayIOStateReader>(jarr);
// 	return curArray.get();
// }

// JsonObjectIOState* JsonObjectIOStateReader::objectBegin(const BackString& name)
// {
// 	const BackJson& jobj = json[name];
// 	curObj = std::make_unique<JsonObjectIOStateReader>(jobj);
// 	return curObj.get();
// }

// JsonArrayIOState* JsonObjectIOStateWriter::arrayBegin(const BackString& name, int& size)
// {
// 	JsonArray jarr(Json::ValueType::arrayValue);
// 	json[name] = jarr;
// 	BackJson& refjarr = json[name];
// 	refjarr.resize(size);
// 	curArray = std::make_unique<JsonArrayIOStateWriter>(refjarr);
// 	return curArray.get();
// }

// JsonObjectIOState* JsonObjectIOStateWriter::objectBegin(const BackString& name)
// {
// 	JsonObject jobj(Json::ValueType::objectValue);
// 	json[name] = jobj;
// 	BackJson& refjobj = json[name];
// 	curObj = std::make_unique<JsonObjectIOStateWriter>(refjobj);
// 	return curObj.get();
// }
