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
		_index, _focallength, _azimut.degrees(), _name.c_str());
}

astro::Angle	ccddata::resolution() const {
	return astro::Angle(_ccdinfo.pixelwidth / _focallength);
}

} // namespace snowgui
