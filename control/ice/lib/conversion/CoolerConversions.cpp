/*
 * CoolerConversions.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <IceConversions.h>

namespace snowstar {

CoolerInfo      convert(const astro::camera::CoolerInfo& ci) {
	CoolerInfo	coolerinfo;
	coolerinfo.actualTemperature = ci.actualTemperature().temperature();
	coolerinfo.setTemperature = ci.setTemperature().temperature();
	coolerinfo.on = ci.on();
	return coolerinfo;
}

astro::camera::CoolerInfo       convert(const CoolerInfo& ci) {
	return astro::camera::CoolerInfo(
			astro::Temperature(ci.actualTemperature),
			astro::Temperature(ci.setTemperature), ci.on);
}

}
