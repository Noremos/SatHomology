#ifdef USE_MODULE
module;
#else
#pragma once
#endif

#include <assert.h>
#include <initializer_list>
#include <iostream>
// Side
#include "PortFileDialog.h"
#include "sago/platform_folders.h"

#ifdef USE_MODULE
export module Platform;
import BackBind;
#define MEXPORT export
#else
#include "Usings.h"
#define MEXPORT
#endif


//#define GL_RGB                            0x1907

MEXPORT static std::vector<BackPathStr> openImagesOrProject()
{
	// File open
	auto f = pfd::open_file("Choose files to read", pfd::path::home(),
		{ "All Files", "*",
		"Images (.png .jpg, .tif)", "*.png *.jpg *.tiff *tif",
		"Project (.qwr)", "*.qwr",
		},
		pfd::opt::multiselect);

	std::cout << "Selected files:";
	for (auto const& name : f.result())
		std::cout << " " + name;
	std::cout << "\n";

	std::vector<BackPathStr> output;
	const auto r = f.result();
	for (auto &s : r)
	{
		output.push_back(s);
	}

	return output;
}

MEXPORT static BackPathStr openImage()
{
	// File open
	auto f = pfd::open_file("Choose files to read", pfd::path::home(),
		{
			"Images (.png .jpg, .tif)", "*.png *.jpg *.tiff *tif",
			"All Files", "*"
		},
		pfd::opt::none);

	std::cout << "Selected files:";
	for (auto const& name : f.result())
		std::cout << " " + name;
	std::cout << "\n";

	return f.result().size() == 0 ? "" : f.result()[0];
}


MEXPORT static BackPathStr openFile(BackStringView filter)
{
	// Split filter by the '|' character
	std::vector<BackString> filters;
	for (auto& s : filter)
	{
		if (s == '|')
		{
			filters.push_back("");
		}
		else
		{
			filters.back() += s;
		}
	}
	if (filters.size() == 0)
	{
		filters.push_back("All Files");
		filters.push_back("*");

	}
	// File open
	auto f = pfd::open_file("Choose files to read", pfd::path::home(),
		filters,
		pfd::opt::none);

	std::cout << "Selected files:";
	for (auto const& name : f.result())
		std::cout << " " + name;
	std::cout << "\n";

	return f.result().size() == 0 ? "" : f.result()[0];
}


MEXPORT static BackPathStr openFolder()
{
	// File open
	auto f = pfd::select_folder("Choose folder to read", pfd::path::home(),
		pfd::opt::none);

	return f.result();
}

MEXPORT static BackPathStr openProject()
{
	// File open
	auto f = pfd::open_file("Choose files to read", pfd::path::home(),
		{
			"Project (.qwr)", "*.qwr",
			"All Files", "*"
		},
		pfd::opt::none);

	std::cout << "Selected files:";
	for (auto const& name : f.result())
		std::cout << " " + name;
	std::cout << "\n";

	return f.result().size() == 0 ? "" : f.result()[0];
}

MEXPORT static BackPathStr getSavePath(std::initializer_list<BackString> exts)
{
	auto fs = pfd::save_file("Choose file to save", pfd::path::home(), exts, pfd::opt::none);

	std::cout << "File path:" << " " + fs.result() << std::endl;

	return fs.result().size() == 0 ? "" : fs.result();
}

MEXPORT static BackPathStr getDicumnetPath()
{
	return sago::getDocumentsFolder();
}

#undef MEXPORT
