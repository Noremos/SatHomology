module;

#include "../Bind/Common.h"
#include <memory>

export module MetadataIOCore;

import JsonCore;
import BarcodeModule;
import IOCore;


export class MetadataProvider
{
	BackDirStr path;
	int& counter;

public:
	MetadataProvider(const BackDirStr& path, int& counter) :
		path(path), counter(counter)
	{ }

	MetadataProvider(const MetadataProvider& meta) :
		path(meta.path), counter(meta.counter)
	{ }


	void update(const MetadataProvider& mpv)
	{
		path = mpv.path;
		counter = mpv.counter;
	}

	BackPathStr getUniquePath(int& id)
	{
		id = counter++;
		return path / intToStr(id);
	}

	int getUniqueId()
	{
		return counter++;
	}

	BackPathStr getUniquePath(int& id, const BackString& ext)
	{
		id = counter++;
		return path / (intToStr(id) + (ext[0] == '.' ? ext : ("." + ext)));
	}

	BackPathStr getPath(int id, const BackString& ext) const
	{
		return path / (intToStr(id) + (ext[0] == '.' ? ext : ("." + ext)));
	}

	BackDirStr getSubFolder(const BackString& name) const
	{
		return path / name;
	}

	BackDirStr getFile(const BackString& name) const
	{
		return path / name;
	}

	MetadataProvider getSubMeta(const BackString& folder) const
	{
		return MetadataProvider(path / folder, counter);
	}

	void mkdir()
	{
		mkDirIfNotExists(path);
	}

// 	MetadataProvider store(const BackImage& img)
// 	{
// 		BackPathStr bps = getUniquePath(".png");
// 		MetadataProvider mp;
// 		mp.path = path / folder;
// 		return mp;
// 	}
};



export class UserdefIO
{
public:
	virtual void writeData(BackJson& json) const = 0;
	virtual void readData(const BackJson& json) = 0;
};


export union SettVariant
{
	int* i;
	float* f;
	double* d;
	BackString* s;
	BackPathStr* p;
};

struct SettingValue
{
	SettVariant data;
	BackString name;

private:
	enum SettVariantType
	{
		sv_int,
		sv_float,
		sv_double,
		sv_str,
		sv_path
	} type;

public:
	SettingValue(const BackString& name, int& val)
	{
		this->name = name;
		data.i = &val;
		type = sv_int;
	}

	SettingValue(const BackString& name, float& val)
	{
		this->name = name;
		data.f = &val;
		type = sv_double;
	}
	SettingValue(const BackString& name, double& val)
	{
		this->name = name;
		data.d = &val;
		type = sv_double;
	}

	SettingValue(const BackString& name, BackString& val)
	{
		this->name = name;
		data.s = &val;
		type = sv_str;
	}
	SettingValue(const BackString& name, BackPathStr& val)
	{
		this->name = name;
		data.p = &val;
		type = sv_path;
	}

	void writeData(BackJson& json) const
	{
		switch (type)
		{
		case sv_int:
			json[name] = *data.i;
			break;
		case sv_float:
			json[name] = *data.f;
			break;
		case sv_double:
			json[name] = *data.d;
			break;
		case sv_str:
			json[name] = *data.s;
			break;
		case sv_path:
			json[name] = data.p->string();
			break;
		}
	}

	void readData(const BackJson& json)
	{
		switch (type)
		{
		case sv_int:
			*data.i = json[name].asInt();
			break;
		case sv_float:
			*data.f = json[name].asFloat();
			break;
		case sv_double:
			*data.d = json[name].asDouble();
			break;
		case sv_str:
			*data.s = json[name].asString();
			break;
		case sv_path:
			*data.p = json[name].asString();
			break;
		}
	}
};

export class SettingsIO
{
	std::vector<SettingValue> settings;
public:
	std::function<void(BackJson& json)> extraWrite;
	std::function<void(const BackJson& json)> extraRead;


	SettingsIO(std::initializer_list<SettingValue> l) : settings(l)
	{ }

	void write(BackJson& json) const
	{
		for (auto& set : settings)
		{
			set.writeData(json);
		}

		if (extraWrite)
			extraWrite(json);
	}
	void read(const BackJson& json)
	{
		for (auto& set : settings)
		{
			set.readData(json);
		}

		if (extraRead)
			extraRead(json);
	}
};

class JsonArrayIOState;
class JsonArrayIOStateReader;
class JsonArrayIOStateWriter;
class JsonObjectIOStateReader;
class JsonObjectIOStateWriter;

export class JsonObjectIOState
{
public:
	virtual bool isReading() const = 0;
	virtual void scInt(const BackString& name, int& val) = 0;
	virtual void scFloat(const BackString& name, float& val) = 0;
	virtual void scDouble(const BackString& name, double& val) = 0;
	virtual void scStr(const BackString& name, BackString& val) = 0;
	virtual void scPath(const BackString& name, BackPathStr& val) = 0;
	virtual JsonArrayIOState* arrayBegin(const BackString& name, int& size) = 0;
	virtual JsonObjectIOState* objectBegin(const BackString& name) = 0;
};

export class JsonArrayIOState
{
public:
	virtual void scUchar(int ind, uchar& val) = 0;
	virtual void scInt(int ind, int& val) = 0;
	virtual void scFloat(int ind, float& val) = 0;
	virtual void scDouble(int ind, double& val) = 0;
	virtual void scStr(int ind, BackString& val) = 0;
	virtual void scPath(int ind, BackPathStr& val) = 0;
	virtual JsonArrayIOState* arrayBegin(int ind, int& size) = 0;
	virtual JsonObjectIOState* objectBegin(int ind) = 0;
};

export class JsonObjectIOStateReader : public JsonObjectIOState
{
	const BackJson& json;
	std::unique_ptr<JsonArrayIOStateReader> curArray;
	std::unique_ptr<JsonObjectIOStateReader> curObj;
public:
	JsonObjectIOStateReader(const BackJson& json) : json(json)
	{}

	bool isReading() const
	{
		return true;
	}

	void scInt(const BackString& name, int& val)
	{
		val = json[name].asInt();
	}
	void scFloat(const BackString& name, float& val)
	{
		val = json[name].asFloat();
	}
	void scDouble(const BackString& name, double& val)
	{
		val = json[name].asDouble();
	}
	void scStr(const BackString& name, BackString& val)
	{
		val = json[name].asString();
	}
	void scPath(const BackString& name, BackPathStr& val)
	{
		val = json[name].asString();
	}

	JsonArrayIOState* arrayBegin(const BackString& name, int& size);
	JsonObjectIOState* objectBegin(const BackString& name);
};


export class JsonObjectIOStateWriter : public JsonObjectIOState
{
	BackJson& json;
	std::unique_ptr<JsonArrayIOStateWriter> curArray;
	std::unique_ptr<JsonObjectIOStateWriter> curObj;

public:
	JsonObjectIOStateWriter(BackJson& json) : json(json)
	{}

	bool isReading() const
	{
		return false;
	}

	void scInt(const BackString& name, int& val)
	{
		json[name] = val;
	}
	void scFloat(const BackString& name, float& val)
	{
		json[name] = val;
	}
	void scDouble(const BackString& name, double& val)
	{
		json[name] = val;
	}
	void scStr(const BackString& name, BackString& val)
	{
		json[name] = val;
	}
	void scPath(const BackString& name, BackPathStr& val)
	{
		json[name] = val.string();
	}

	JsonArrayIOState* arrayBegin(const BackString& name, int& size);
	JsonObjectIOState* objectBegin(const BackString& name);
};


export class JsonArrayIOStateReader : public JsonArrayIOState
{
	const BackJson& json;
	std::unique_ptr<JsonArrayIOStateReader> curArray;
	std::unique_ptr<JsonObjectIOStateReader> curObj;
public:
	JsonArrayIOStateReader(const BackJson& json) : json(json)
	{}

	void scUchar(int ind, uchar& val)
	{
		val = static_cast<uchar>(json[ind].asUInt());
	}

	void scInt(int ind, int& val)
	{
		val = json[ind].asInt();
	}
	void scFloat(int ind, float& val)
	{
		val = json[ind].asFloat();
	}
	void scDouble(int ind, double& val)
	{
		val = json[ind].asDouble();
	}
	void scStr(int ind, BackString& val)
	{
		val = json[ind].asString();
	}
	void scPath(int ind, BackPathStr& val)
	{
		val = json[ind].asString();
	}

	JsonArrayIOState* arrayBegin(int ind, int& size)
	{
		const BackJson& jarr = json[ind];
		size = jarr.size();
		curArray = std::make_unique<JsonArrayIOStateReader>(jarr);
		return curArray.get();
	}

	JsonObjectIOState* objectBegin(int ind)
	{
		const BackJson& jarr = json[ind];
		curObj = std::make_unique<JsonObjectIOStateReader>(jarr);
		return curObj.get();
	}
};

export class JsonArrayIOStateWriter : public JsonArrayIOState
{
	BackJson& json;
	std::unique_ptr<JsonArrayIOStateWriter> curArray;
	std::unique_ptr<JsonObjectIOStateWriter> curObj;

public:
	JsonArrayIOStateWriter(BackJson& json) : json(json)
	{}

	void scUchar(int ind, uchar& val)
	{
		json[ind] = val;
	}

	void scInt(int ind, int& val)
	{
		json[ind] = val;
	}
	void scFloat(int ind, float& val)
	{
		json[ind] = val;
	}
	void scDouble(int ind, double& val)
	{
		json[ind] = val;
	}
	void scStr(int ind, BackString& val)
	{
		json[ind] = val;
	}
	void scPath(int ind, BackPathStr& val)
	{
		json[ind] = val.string();
	}

	JsonArrayIOState* arrayBegin(int ind, int& size)
	{
		JsonArray jarr(Json::ValueType::arrayValue);
		json[ind] = jarr;
		BackJson& refjarr = json[ind];
		refjarr.resize(size);
		curArray = std::make_unique<JsonArrayIOStateWriter>(refjarr);
		return curArray.get();
	}

	JsonObjectIOState* objectBegin(int ind)
	{
		JsonArray jarr(Json::ValueType::objectValue);
		json[ind] = jarr;
		BackJson& refjarr = json[ind];
		curObj = std::make_unique<JsonObjectIOStateWriter>(refjarr);
		return curObj.get();
	}

	JsonObjectIOStateWriter* append()
	{
		curObj = std::make_unique<JsonObjectIOStateWriter>(json.append(BackJson()));
		return curObj.get();
	}
};


JsonArrayIOState* JsonObjectIOStateReader::arrayBegin(const BackString& name, int& size)
{
	const BackJson& jarr = json[name];
	size = jarr.size();
	curArray = std::make_unique<JsonArrayIOStateReader>(jarr);
	return curArray.get();
}

JsonObjectIOState* JsonObjectIOStateReader::objectBegin(const BackString& name)
{
	const BackJson& jarr = json[name];
	curObj = std::make_unique<JsonObjectIOStateReader>(jarr);
	return curObj.get();
}

JsonArrayIOState* JsonObjectIOStateWriter::arrayBegin(const BackString& name, int& size)
{
	JsonArray jarr(Json::ValueType::arrayValue);
	json[name] = jarr;
	BackJson& refjarr = json[name];
	refjarr.resize(size);
	curArray = std::make_unique<JsonArrayIOStateWriter>(refjarr);
	return curArray.get();
}

JsonObjectIOState* JsonObjectIOStateWriter::objectBegin(const BackString& name)
{
	JsonObject jarr(Json::ValueType::objectValue);
	json[name] = jarr;
	BackJson& refjarr = json[name];
	curObj = std::make_unique<JsonObjectIOStateWriter>(refjarr);
	return curObj.get();
}


export class IJsonIO
{
public:
	void read(const JsonObject& obj, MetadataProvider& metaFolder)
	{
		JsonObjectIOStateReader state(obj);
		saveLoadState(&state, metaFolder);
	}

	void write(JsonObject& obj, MetadataProvider& metaFolder)
	{
		JsonObjectIOStateWriter state(obj);
		saveLoadState(&state, metaFolder);
	}
private:
	virtual void saveLoadState(JsonObjectIOState* state, MetadataProvider& metaFolder) = 0;
};


export class IBffIO
{
public:
	virtual void read(StateBinFile::BinState* state)
	{
		saveLoadState(state);
	}
	void read(std::istream& stream)
	{
		StateBinFile::BinStateReader reader(stream);
		saveLoadState(&reader);
	}

	void write(StateBinFile::BinState* state)
	{
		saveLoadState(state);
	}
	void write(std::ostream& stream)
	{
		StateBinFile::BinStateWriter writer(stream);
		saveLoadState(&writer);
	}

private:
	virtual void saveLoadState(StateBinFile::BinState* state) = 0;
};
