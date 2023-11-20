module;
#include "../../Bind/Common.h"

export module MLSettings;

import JsonCore;


export union OptionVariant
{
	bool b;
	int i;
	double d;
	BackString* s;
	BackPathStr* p;
};


export struct OptionValue
{
	OptionVariant data;
	BackString name;

	enum OptionVariantType
	{
		sv_bool,
		sv_int,
		sv_double,
		sv_str,
		sv_path
	} type;

public:
	OptionValue(const BackString& name, bool val)
	{
		this->name = name;
		data.b = val;
		type = sv_bool;
	}

	OptionValue(const BackString& name, int val)
	{
		this->name = name;
		data.i = val;
		type = sv_int;
	}

	OptionValue(const BackString& name, double val)
	{
		this->name = name;
		data.d = val;
		type = sv_double;
	}

	OptionValue(const BackString& name, BackString val)
	{
		this->name = name;
		data.s = new BackString(val);
		type = sv_str;
	}
	OptionValue(const BackString& name, BackPathStr val)
	{
		this->name = name;
		data.p = &val;
		type = sv_path;
	}
	~OptionValue()
	{
		switch (type)
		{
		case OptionValue::sv_str:
			delete data.s;
			break;
		case OptionValue::sv_path:
			delete data.p;
			break;
		default:
			break;
		}
	}

	void writeData(BackJson& json) const
	{
		switch (type)
		{
		case sv_bool:
			json[name] = data.b;
			break;
		case sv_int:
			json[name] = data.i;
			break;
		case sv_double:
			json[name] = data.d;
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
			data.b = json[name].asBool();
			break;
		case sv_int:
			data.i = json[name].asInt();
			break;
		case sv_double:
			data.d = json[name].asDouble();
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

export class MLSettings
{
public:
	std::vector<OptionValue> values;

	bool* getBool(BackStringView name)
	{
		for (auto& value : values)
		{
			if (value.name == name)
			{
				return &value.data.b;
			}
		}

		return nullptr;
	}

	BackString* getString(BackStringView name)
	{
		for (auto& value : values)
		{
			if (value.name == name)
			{
				return value.data.s;
			}
		}

		return nullptr;
	}

	int* getInt(BackStringView name)
	{
		for (auto& value : values)
		{
			if (value.name == name)
			{
				return &value.data.i;
			}
		}

		return nullptr;
	}

	double* getDouble(BackStringView name)
	{
		for (auto& value : values)
		{
			if (value.name == name)
			{
				return &value.data.d;
			}
		}

		return nullptr;
	}

	MLSettings(std::initializer_list<OptionValue> l = {}) : values(l)
	{ }

	void write(BackJson& json) const
	{
		for (auto& set : values)
		{
			set.writeData(json);
		}
	}
	void read(const BackJson& json)
	{
		for (auto& set : values)
		{
			set.readData(json);
		}
	}
};
