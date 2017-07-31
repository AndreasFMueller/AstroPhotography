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

long	SimFocuser::variance() {
	return (max() - min()) / 4;
}

SimFocuser::SimFocuser(SimLocator& locator)
	: Focuser(DeviceName("focuser:simulator/focuser")),
	  _locator(locator) {
	_value = /* 10000 + */ (max() + min()) / 2;
debug(LOG_DEBUG, DEBUG_LOG, 0, "focuser set to %d", _value);
	target = _value;
	lastset = 0;
}

void	SimFocuser::randomposition() {
	_value = reference() + (random() % variance());
	target = _value;
	lastset = 0;
}

long	SimFocuser::min() {
	return 0;
}

long	SimFocuser::max() {
	return 100000;
}

long	SimFocuser::current() {
	if (0 == lastset) {
		return _value;
	}
	double	now = simtime();
	double	timepast = now - lastset;
	double	delta = (double)_value - (double)target;
debug(LOG_DEBUG, DEBUG_LOG, 0, "delta: %f, timepast: %f", delta, timepast);
	if (fabs(delta / 1000.) > timepast) {
		_value -= timepast * delta;
		lastset = now;
	} else {
		lastset = 0;
		_value = target;
	}
	return _value;
}

long	SimFocuser::backlash() {
	return 1000;
}

void	SimFocuser::set(long value) {
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "radius = %f", MAXRADIUS * r);
	return MAXRADIUS * r;
}

} // namespace simulator
} // namespace camera
} // namespace astro
