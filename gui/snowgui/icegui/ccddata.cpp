/*
 * ccddata.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "ccdcontrollerwidget.h"

namespace snowgui {

/**
 * \brief convert ccddata to a string
 */
std::string     ccddata::toString() const {
	return astro::stringprintf("%s[%d] f=%.3f, azi=%.2f, %s",
		(_type == snowstar::InstrumentCCD) ? "imaging" :
		(_type == snowstar::InstrumentFinderCCD) ? "finder" : "guider",
		_index, _focallength, _azimuth.degrees(), _name.c_str());
}

astro::Angle	ccddata::resolution() const {
	return astro::Angle(_ccdinfo.pixelwidth / _focallength);
}

ImagerRectangle	ccddata::imagerrectangle() const {
	ImagerRectangle	result;
	result.azimuth(_azimuth);
	astro::Angle	_res = resolution();
	astro::TwoAngles	size;
	size.a1() = _ccdinfo.size.width * _res;
	size.a2() = _ccdinfo.size.height * _res;
	result.size(size);
	return result;
}

} // namespace snowgui
