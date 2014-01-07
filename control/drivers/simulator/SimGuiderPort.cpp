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
 *
 * The default settings of the guider port have a coordinate system rotated
 * by 30 degrees with respect to the ccd axes. Also the vector in the right
 * ascension direction is shorter, approximately as if declination was
 * 45 degrees.
 */
SimGuiderPort::SimGuiderPort(SimLocator& locator)
	: GuiderPort("guiderport:simulator/guiderport"), _locator(locator) {
	starttime = simtime();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "SimGuiderPort created at %f",
		starttime);
	_omega = 0;
	// the initial mount axis directions are not parallel to the coordinate
	// axes of the image
	_ravector = sqrt(0.5) * Point(sqrt(3) / 2, 0.5);
	_decvector = Point(-0.5, sqrt(3) / 2);
	ra = 0;
	dec = 0;
}

/**
 * \brief Update the offset to the current time
 *
 * 
 */
void	SimGuiderPort::update() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guider port @ %p", this);
	// if this is the first 
	if ((ra == 0) && (dec == 0)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no update");
		return;
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "update: current offset: %s",
		_offset.toString().c_str());

	// advance the offset according to last activation
	double	now = simtime();
	double	activetime = now - lastactivation;
	if (fabs(ra) < activetime) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "update: advance RA by %f", ra);
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
		debug(LOG_DEBUG, DEBUG_LOG, 0, "update: remaining RA activation: %f",
			ra);
	}
	if (dec < activetime) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "update: advance DEC by %f", dec);
		_offset = _offset + dec * _decvector;
		dec = 0;
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "update: advance DEC by %f", activetime);
		_offset = _offset + activetime * _decvector;
		if (dec > 0) {
			dec -= activetime;
		} else {
			dec += activetime;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "update: remaining DEC activation: %f",
			dec);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "update: new offset: %s",
		_offset.toString().c_str());
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
		" decplus = %.3f, decminus = %.3f)",
		raplus, raminus, decplus, decminus);
	if ((raplus < 0) || (raminus < 0) || (decminus < 0) || (decplus < 0)) {
		throw BadParameter("activation times must be nonegative");
	}

	// update the offset
	update();
	
	// perform this new activation
	lastactivation = simtime();
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new activations: ra = %f, dec = %f",
		ra, dec);
}

/**
 * \brief Retrieve the current offset
 */
Point	SimGuiderPort::offset() {
	double	timepast = simtime() - starttime;

	// drift computation
	Point	p = timepast * _drift;
debug(LOG_DEBUG, DEBUG_LOG, 0, "drift: %s", p.toString().c_str());

	// Fourier components
	if (timepast > 360) {
		double	angle = 0.01 * timepast;
		Point	fourier = 5. * Point(sin(angle), cos(angle));
		p = p + fourier;
	}

	// return the point
debug(LOG_DEBUG, DEBUG_LOG, 0, "complete offset: %s", (_offset + p).toString().c_str());
	return _offset + p;
}

double	SimGuiderPort::alpha() {
	return (simtime() - starttime) * _omega;
}

} // namespace simulator
} // namespace camera
} // namespace astro
