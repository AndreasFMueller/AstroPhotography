/*
 * GainControl.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <Control.h>

namespace astro {
namespace guiding {

GainControl::GainControl(double deltat) : ControlBase(deltat) {
}

GainControl::~GainControl() {
}

Point	GainControl::correct(const Point& offset) {
	Point	corrected = ControlBase::correct(offset)
				* Point(gain(0), gain(1));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "gain corrected: %s -> %s",
		offset.toString().c_str(), corrected.toString().c_str());
	return corrected;
}

} // namespace guiding
} // namespace astro
