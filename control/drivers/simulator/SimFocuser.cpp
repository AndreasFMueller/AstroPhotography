/*
 * SimFocuser.cpp -- 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimFocuser.h>
#include <limits>
#include <SimUtil.h>

namespace astro {
namespace camera {
namespace simulator {

double	SimFocuser::reference() {
	double	r = min();
	r += max();
	return r / 2.;
}

unsigned short	SimFocuser::variance() {
	return std::numeric_limits<unsigned short>::max() / 4;
}

SimFocuser::SimFocuser(SimLocator& locator) : _locator(locator) {
	_value = reference() + (random() % variance());
	target = _value;
	lastset = 0;
}

unsigned short	SimFocuser::min() {
	return 0;
}

unsigned short	SimFocuser::max() {
	return std::numeric_limits<unsigned short>::max();
}

unsigned short	SimFocuser::current() {
	if (0 == lastset) {
		return _value;
	}
	double	now = simtime();
	double	timepast = now - lastset;
	if (fabs((_value - target) / 1000.) > timepast) {
		_value -= timepast * (_value - target) / 1000.;
		lastset = now;
	} else {
		lastset = 0;
		target = _value;
	}
	return _value;
}

void	SimFocuser::set(unsigned short value) {
	current();
	if (value == target) {
		return;
	}
	lastset = simtime();
	target = value;
}

#define	MAXRADIUS	20

double	SimFocuser::radius() {
	double	r = fabs((reference() - current()) / (double)variance());
	return MAXRADIUS * r;
}

} // namespace simulator
} // namespace camera
} // namespace astro
