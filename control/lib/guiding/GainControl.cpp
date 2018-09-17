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
	return ControlBase::correct(offset) * Point(gain(0), gain(1));
}

} // namespace guiding
} // namespace astro
