module;
#include "Barcode/PrjBarlib/include/barline.h"
#include "Common.h"

#include "../MatrImg.h"
#include "../Clusterizers/MLSettings.h"
export module AlgUtils;

// import MLSettings;
import RasterLayers;
import ProjectModule;
//import BackBind;
// import MatrModule;
import LayersCore;

export MLSettings mkSettingsType()
{
	MLSettings settings;
	OptionValue comp("type", {});
	comp.data.e->add("От 0 до 255", bc::ProcType::f0t255);
	comp.data.e->add("От 255 до 0", bc::ProcType::f255t0);
	comp.data.e->add("По расстоянию", bc::ProcType::Radius);
	settings.values.push_back(comp);

	return settings;
}


export Barscalar lerp(double t)
{
	// if (t < 0)
	// 	t = 0;

	assert(t <= 1.0 && t >= 0);
	//t = log(1 + t);
	const BackColor start(0, 255, 255);
	const BackColor end(255, 0, 0);
	uint8_t r = static_cast<uint8_t>(start.r + t * (end.r - start.r));
	uint8_t g = static_cast<uint8_t>(start.g + t * (end.g - start.g));
	uint8_t b = static_cast<uint8_t>(start.b + t * (end.b - start.b));
	return Barscalar(r, g, b);
}


export bc::barstruct getConstr(const MLSettings& setting)
{
	bc::ProcType type = setting.getEnumValue<bc::ProcType>("type");
	const int* energyStart = setting.getInt("energyStart");

	bc::barstruct constr;
	constr.createBinaryMasks = true;
	constr.createGraph = false;
	constr.attachMode = bc::AttachMode::morePointsEatLow;
	//constr.attachMode = bc::AttachMode::closer;
	constr.returnType = bc::ReturnType::barcode2d;
	if (energyStart)
		constr.energyStart = *energyStart;

	constr.addStructure(type, bc::ColorType::native, bc::ComponentType::Component);

	return constr;
}


export BackSize getSrcFromInput(InOutLayer iol, BackImage& out, std::optional<BackSize> newSize = {})
{
	Project* proj = Project::getProject();
	IRasterLayer* input = proj->getInRaster(iol);

	const BackImage& srcl = *(input->getCachedImage());
	const BackSize srcsize = newSize.value_or(BackSize(srcl.width(), srcl.height()));
	float aspect = 1.f;

	out = BackImage(srcsize.wid, srcsize.hei, input->getRect(0, 0, 1, 1).get(0, 0).type);
	aspect = static_cast<float>(input->realWidth()) / srcsize.wid;
	if (input->displayWidth() != srcsize.wid)
	{
		for (int h = 0; h < srcsize.hei; ++h)
		{
			for (int i = 0; i < srcsize.wid; ++i)
			{
				out.set(i, h, input->getRect(i * aspect, h * aspect, 1, 1).get(0, 0));
			}
		}
	}
	else
		out = srcl;

	return srcsize;
}


export RasterLayer* genOutputFromInput(InOutLayer iol, BackImage& out, std::optional<BackSize> newSize = {})
{
	BackSize srcsize = getSrcFromInput(iol, out, newSize);

	Project* proj = Project::getProject();
	IRasterLayer* input = proj->getInRaster(iol);
	float aspect = static_cast<float>(input->realWidth()) / srcsize.wid;

	RasterLayer* rasterSpot = proj->addLayerData<RasterLayer>(input->cs.getProjId());
	rasterSpot->initCSFrom(input->cs);
	rasterSpot->aspect = aspect;
	rasterSpot->init(srcsize.wid, srcsize.hei, 3);

	return rasterSpot;
}
