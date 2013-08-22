/*
 * SimGuiderPort.cpp -- Guider Port implementation for simulator camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimGuiderPort.h>
#include <SimUtil.h>
#include <AstroDebug.h>
#include <AstroExceptions.h>

namespace astro {
namespace camera {
namespace simulator {

/**
 * \brief Create a simulated GuiderPort
 */
SimGuiderPort::SimGuiderPort(SimLocator& locator)
	: GuiderPort("sim-guiderport"), _locator(locator) {
	starttime = simtime();
	_omega = 0;
	_decvector = Point(0, 1);
	_ravector = Point(1, 0);
	ra = 0;
	dec = 0;
}

/**
 * \brief Update the offset to the current time
 *
 * 
 */
void	SimGuiderPort::update() {
	// if this is the first 
	if ((ra == 0) && (dec == 0)) {
		return;
	}

	// advance the offset according to last activation
	double	now = simtime();
	double	activetime = now - lastactivation;
	if (fabs(ra) < activetime) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "advance RA by %f", ra);
		_offset = _offset + ra * _ravector;
		ra = 0;
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "advance RA by %f", activetime);
		_offset = _offset + activetime * _ravector;
		if (ra > 0) {
			ra -= activetime;
		} else {
			ra += activetime;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "remaining RA activation: %f",
			ra);
	}
	if (dec < activetime) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "advance DEC by %f", dec);
		_offset = _offset + dec * _decvector;
		dec = 0;
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "advance DEC by %f", activetime);
		_offset = _offset + activetime * _decvector;
		if (dec > 0) {
			dec -= activetime;
		} else {
			dec += activetime;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "remaining DEC activation: %f",
			dec);
	}
}

/**
 * \brief report which outputs are active
 */
uint8_t	SimGuiderPort::active() {
	update();
	uint8_t	result = 0;
	if (ra > 0) {
		result |= RAPLUS;
	}
	if (ra < 0) {
		result |= RAMINUS;
	}
	if (dec > 0) {
		result |= DECPLUS;
	}
	if (dec < 0) {
		result |= DECMINUS;
	}
	return 0;
}

/**
 * \brief Activate the guider port outputs
 */
void	SimGuiderPort::activate(float raplus, float raminus,
		float decplus, float decminus) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "activate(raplus = %.3f, raminus = %.3f,"
		" decplus = %.3f, decminus = %.3f",
		raplus, raminus, decplus, decminus);
	if ((raplus < 0) || (raminus < 0) || (decminus < 0) || (decplus < 0)) {
		throw BadParameter("activation times must be nonegative");
	}

	// update the offset
	update();
	
	// perform this new activation
	lastactivation = 0;
	if (raplus > 0) {
		ra = raplus;
	} else {
		ra = -raminus;
	}
	if (decplus > 0) {
		dec = decplus;
	} else {
		dec = -decminus;
	}
}

/**
 * \brief Retrieve the current offset
 */
Point	SimGuiderPort::offset() {
	double	x = _offset.x(), y = _offset.y();
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
