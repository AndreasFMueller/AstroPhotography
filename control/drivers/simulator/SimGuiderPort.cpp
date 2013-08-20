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
	_driftx = 0;
	_drifty = 0;
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

std::pair<double, double>	SimGuiderPort::offset() {
	double	x = 0, y = 0;
	double	timepast = simtime() - starttime;

	// drift computation
	x += _driftx * timepast;
	y += _drifty * timepast;

	// rotation
	double	alpha = _omega * timepast;
	double	xx = x * cos(alpha) - y * sin(alpha);
	y = x * sin(alpha) + y * cos(alpha);
	x = xx;

	// XXX Fourier components

	return std::make_pair(x, y);
}

} // namespace simulator
} // namespace camera
} // namespace astro
