module;

#include <assert.h>
#include <initializer_list>
#include <iostream>
// Side
#include "PortFileDialog.h"
#include "sago/platform_folders.h"
#include "Usings.h"
export module Platform;

import BackBind;
import BackTypes;


//#define GL_RGB                            0x1907

export BackPathStr openImageOrProject()
{
	// File open
	auto f = pfd::open_file("Choose files to read", pfd::path::home(),
		{ "All Files", "*",
		"Images (.png .jpg, .tif)", "*.png *.jpg *.tiff *tif",
		"Project (.qwr)", "*.qwr",
		},
		pfd::opt::none);

	std::cout << "Selected files:";
	for (auto const& name : f.result())
		std::cout << " " + name;
	std::cout << "\n";

	return f.result().size() == 0 ? "" : f.result()[0];
}

export BackPathStr openImage()
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


export BackPathStr openProject()
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

export BackPathStr getSavePath(std::initializer_list<BackString> exts)
{
	auto fs = pfd::save_file("Choose file to save", pfd::path::home(), exts, pfd::opt::none);

	std::cout << "File path:" << " " + fs.result() << std::endl;

	return fs.result().size() == 0 ? "" : fs.result();
}

export BackPathStr getDicumnetPath()
{
	return sago::getDocumentsFolder();
}
