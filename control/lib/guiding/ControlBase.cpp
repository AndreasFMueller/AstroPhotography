/*
 * ControlBase.cpp -- base class for control algorithm
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <Control.h>

namespace astro {
namespace guiding {

ControlBase::ControlBase(CalibrationPtr cal, double deltat)
	: _calibration(cal), _deltat(deltat) {
}

Point	ControlBase::correct(const Point& offset) {
	return _calibration->correction(offset, deltat());
}

} // namespace guiding
} // namespace astro
