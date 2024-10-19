#include <algorithm>
#include <cmath>
#include <memory>
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include <random>


#include "../MatrImg.h"
#include "../MLSettings.h"
#include "../Layers/layerInterface.h"
#include "../Layers/Rasterlayers.h"
#include "../backend/Interfaces/IAlgorithm.h"
#include "../backend/Algs/SimpleBar.h"

// import MLSettings;
// import MatrModule;
//import IAlgorithm
//import AlgUtils;
//import SimpleBar;


RetLayers exeBargraph(InOutLayer iol, const MLSettings& setting)
{
	bc::barstruct constr = getConstr(setting);
	constr.coltype = bc::ColorType::gray;

	using namespace std::string_literals;
	BackImage main;

	getSrcFromInput(iol, main);

	std::unique_ptr<bc::Baritem> citem(bc::BarcodeCreator::create(main, constr));


	StateBinFile::BinStateWriter writer;
	writer.open("temp.bin");


	auto& lines = citem->barlines;
	writer.pArray(static_cast<buint>(lines.size()));
	for(bc::barline* line : lines)
	{
		writer.pInt(line->start.getAvgUchar());
		writer.pInt(line->m_end.getAvgUchar());
	}


	return {};
}

static AlgFuncRegister registerExeRestore("DisplayBarGraph", exeBargraph, mkSettingsType, "Preview");
