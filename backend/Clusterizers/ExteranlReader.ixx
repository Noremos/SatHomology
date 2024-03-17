module;

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <iostream>
#include <string>
#include <algorithm>
#include "../side/subprocess.h"
#include "Usings.h"

export module ExteranlReader;

import MLSettings;
import BackBind;

export BackString get_temp_file_path()
{
#ifdef _WIN32
	char buffer[MAX_PATH];
	GetTempPathA(MAX_PATH, buffer);
	BackString temp_path(buffer);
	return temp_path.append("input_array.txt");
#else
	return "/tmp/input_array.txt";
#endif
}


export const char* getPythonExe()
{
#ifdef _WIN32
		return "python.exe";
#else
		return "/usr/bin/python3";
#endif
}

export BackString getPythonSettings(const MLSettings& settings)
{
	BackString json = "{";
	for (auto& set : settings.values)
	{
		if (set.name == "method")
			continue;
		const BackStringView quote = "\"";
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
			json += set.data.e->getSelectedName();
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


export bool exec(const std::vector<BackString>& cmd, std::vector<unsigned long>& assigments, int& n)
{
	std::vector<const char*> args;
	for (size_t i = 0; i < cmd.size(); i++)
	{
		args.push_back(cmd[i].c_str());
		std::cout << cmd[i] << " ";
	}
	args.push_back(NULL);

	std::cout << std::endl;

	std::string outputString;
	struct subprocess_s subprocess;
	int result = subprocess_create(static_cast<const char**>(&args[0]), 0, &subprocess);
	if (0 != result) {
		throw std::runtime_error("subprocess_create failed!");
	}

	char buffer[256];
	FILE* p_stdout = subprocess_stdout(&subprocess);
	while (!feof(p_stdout))
	{
		if (fgets(buffer, sizeof(buffer), p_stdout) != nullptr)
		{
			outputString += buffer;
		}
	}

	if (outputString.length() == 0)
	{
		std::cout << "Something wrong, the output is empty" << std::endl;
		return false;
	}

	if (outputString.length() > 1 && outputString[0] == 'E' && outputString[1] == 'R')
	{
		std::cout << "An error occurred: " << std::endl << outputString << std::endl;
		return false;
	}
	else
	{
		std::cout << outputString << std::endl;
	}

	outputString.resize(outputString.length() - 1); // Skip last '\n'
	BackFileReader iss;
	iss.open(outputString);
	iss >> n;
	printf("%d\n", n);
	int number;
	assigments.clear();
	while (iss >> number)
		assigments.push_back(number);

	return true;
}
