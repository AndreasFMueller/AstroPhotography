/*
 * SimGuiderPort.cpp -- Guider Port implementation for simulator camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimGuiderPort.h>
#include <SimUtil.h>
#include <AstroDebug.h>

namespace astro {
namespace camera {
namespace simulator {

SimGuiderPort::SimGuiderPort(SimLocator& locator)
	: GuiderPort("sim-guiderport"), _locator(locator) {
	starttime = simtime();
	_omega = 0;
}

uint8_t	SimGuiderPort::active() {
	return 0;
}

void	SimGuiderPort::activate(float raplus, float raminus,
		float decplus, float decminus) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "activate(raplus = %.3f, raminus = %.3f,"
		" decplus = %.3f, decminus = %.3f",
		raplus, raminus, decplus, decminus);
}

Point	SimGuiderPort::offset() {
	double	x = 0, y = 0;
	double	timepast = simtime() - starttime;

	// drift computation
	x += _drift.x() * timepast;
	y += _drift.y() * timepast;

	// XXX Fourier components

	// return the point
	return Point(x, y);
}

double	SimGuiderPort::alpha() {
	return (simtime() - starttime) * _omega;
}

} // namespace simulator
} // namespace camera
} // namespace astro
