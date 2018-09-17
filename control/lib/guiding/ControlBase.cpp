/*
 * ControlBase.cpp -- base class for control algorithm
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <Control.h>

namespace astro {
namespace guiding {

ControlBase::ControlBase(double deltat) : _deltat(deltat) {
}

ControlBase::~ControlBase() {
}

Point	ControlBase::correct(const Point& offset) {
	return offset;
}

} // namespace guiding
} // namespace astro
