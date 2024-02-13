module;

#include <memory>
#include <functional>
#include <istream>
#include <ostream>
#include "../Bind/Json.h"
#include "Usings.h"
export module MetadataCoreIO;

// import JsonCore;
import StateBinIO;
import BackBind;
export import JsonState;


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

	BackPathStr getUniquePath(int& id) const
	{
		id = counter++;
		return path / intToStr(id);
	}

	int getUniqueId() const
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
	bool* b;
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
		sv_bool,
		sv_int,
		sv_float,
		sv_double,
		sv_str,
		sv_path
	} type;

public:
	SettingValue(const BackString& name, bool& val)
	{
		this->name = name;
		data.b = &val;
		type = sv_bool;
	}

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
		case sv_bool:
			json[name] = *data.b;
			break;
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
		case sv_bool:
			*data.b = json[name].asBool();
			break;
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

export void ioPoint(JsonArrayIOState* state, int id, BackPoint& p)
{
	auto* obj = state->objectBegin(id);
	obj->scDouble("x", p.x);
	obj->scDouble("y", p.y);
	state->objectEnd();
}

///

export class IJsonIO
{
public:
	void read(const JsonObject& obj, const MetadataProvider& metaFolder)
	{
		JsonObjectIOStateReader state(obj);
		saveLoadState(&state, metaFolder);
	}

	void write(JsonObject& obj, const MetadataProvider& metaFolder)
	{
		JsonObjectIOStateWriter state(obj);
		saveLoadState(&state, metaFolder);
	}
private:
	virtual void saveLoadState(JsonObjectIOState* state, const MetadataProvider& metaFolder) = 0;
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
