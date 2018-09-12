/*
 * OptimalControl.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <Control.h>

namespace astro {
namespace guiding {

OptimalControl::OptimalControl(CalibrationPtr cal, double deltat)
	: ControlBase(cal, deltat)  {
}

Point	OptimalControl::correct(const Point& offset) {
	throw std::runtime_error("optimal control filter not implemented yet");
	//return ControlBase::correct(offset);
}

} // namespace guiding
} // namespace astro
