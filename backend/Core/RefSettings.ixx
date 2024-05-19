module;

#include <memory>
#include <functional>
#include <istream>
#include <ostream>
#include "../Bind/Json.h"
#include "Usings.h"

export module RefSettings;

// import JsonCore;
import StateBinIO;
import BackBind;


export union SettVariant
{
	bool* b;
	int* i;
	float* f;
	double* d;
	BackString* s;
	BackPathStr* p;
};

export struct SettingValue
{
	SettVariant data;
	BackString name;

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

export class RefSettings
{
public:
	std::vector<SettingValue> values;

	std::function<void(BackJson& json)> extraWrite;
	std::function<void(const BackJson& json)> extraRead;

	RefSettings(std::initializer_list<SettingValue> l = {}) : values(l)
	{ }

	void add(std::initializer_list<SettingValue> l)
	{
		values.insert(values.end(), l.begin(), l.end());
	}

	void add(const SettingValue& l)
	{
		values.push_back(l);
	}

	void write(BackJson& json) const
	{
		for (auto& set : values)
		{
			set.writeData(json);
		}

		if (extraWrite)
			extraWrite(json);
	}
	void read(const BackJson& json)
	{
		for (auto& set : values)
		{
			set.readData(json);
		}

		if (extraRead)
			extraRead(json);
	}
};