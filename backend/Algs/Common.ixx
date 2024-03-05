module;
#include "Barcode/PrjBarlib/include/barline.h"

export module AlgUtils;

import MLSettings;

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
