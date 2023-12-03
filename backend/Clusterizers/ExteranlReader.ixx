module;

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "../../Bind/Common.h"
#include <iostream>

export module ExteranlReader;

import MLSettings;

export BackString get_temp_file_path()
{
#ifdef _WIN32
	char buffer[MAX_PATH];
	GetTempPathA(MAX_PATH, buffer);
	BackString temp_path(buffer);
	return temp_path + "input_array.txt";
#else
	return "/tmp/input_array.txt";
#endif
}



export BackString getPythonSettings(const MLSettings& settings)
{
	BackString json = "{";
	for (auto& set : settings.values)
	{
		if (set.name == "method")
			continue;
		const BackStringView quote = "\\\"";
		json += quote;
		json += set.name;
		json += quote;
		json += ":";

		BackStringView label = set.name;
		switch (set.type)
		{
		case OptionValue::sv_bool:

			if (set.data.b)
				json += "true";
			else
				json += "false";
			break;
		case OptionValue::sv_int:
			json += std::to_string(set.data.i);
			break;
		case OptionValue::sv_double:
		{
			auto s = std::to_string(set.data.d);
			replace_if(s.begin(), s.end(), [](char c) {return c == ','; }, '.');
			json += s;

			break;
		}
		case OptionValue::sv_str:
			json += quote;
			json += *set.data.s;
			json += quote;
			break;
		case OptionValue::sv_enum:
			json += quote;
			json += set.data.e->getSelected();
			json += quote;
			break;
		case OptionValue::sv_path:
			json += quote;
			json += set.data.p->string();
			json += quote;
			break;
		}

		json += ",";
	}
	json.back() = '}';
	//settings.
	return json;
}


export bool exec(BackStringView cmd, std::vector<unsigned long>& assigments, int& n)
{
	std::cout << cmd << std::endl;
	std::string outputString;
	std::shared_ptr<FILE> pipe(_popen(cmd.data(), "rt"), _pclose);
	if (!pipe) throw std::runtime_error("_popen() failed!");

	char buffer[128];
	while (!feof(pipe.get()))
	{
		if (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr)
		{
			outputString += buffer;
		}
	}

	printf(outputString.c_str());
	if (outputString.length() == 0)
		return false;

	if (outputString.length() > 1 && outputString[0] == 'E' && outputString[1] == 'R')
	{
		return false;
	}

	outputString.resize(outputString.length() - 1); // Skip last '\n'
	std::ifstream iss;
	iss.open(outputString);
	iss >> n;
	printf("%d\n", n);
	int number;
	assigments.clear();
	while (iss >> number)
		assigments.push_back(number);

	return true;
}
