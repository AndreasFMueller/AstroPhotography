/*
 * SimAdaptiveOptics.cpp -- implementation of the adaptive optics simulator
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimAdaptiveOptics.h>
#include <AstroDebug.h>
#include <SimUtil.h>

namespace astro {
namespace camera {
namespace simulator {

using namespace astro::camera;

/**
 * \brief Create an Adaptive Optics simulator unit
 */
SimAdaptiveOptics::SimAdaptiveOptics()
	: AdaptiveOptics("adaptiveoptics:simulator/adaptiveoptics"),
	  pixels_fullrange(16.) {
	starttime = simtime();
	_amplitude = pixels_fullrange / 4;
	_activated = false;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "AdaptiveOptics %s created at %f",
		name().toString().c_str(), starttime);
}

/**
 * \brief Destroy the adaptive optics simulator unit
 */
SimAdaptiveOptics::~SimAdaptiveOptics() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy adaptive optics %s",
		name().toString().c_str());
}

/** 
 * \brief Set position of the adaptive optics unit
 *
 * \param position	position to set the simulated AO device to
 */
void	SimAdaptiveOptics::set0(const Point& position) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set position of %s to %s",
		name().toString().c_str(), position.toString().c_str());
	if (position != Point()) {
		_activated = true;
	}
}

#define	degrees	* (M_PI / 180)
#define	alpha	27 degrees

/**
 * \brief Update the offset to the current time
 */
Point	SimAdaptiveOptics::offset() const {
	if (!_activated) {
		return Point();
	}
	double	age = simtime() - starttime;
	Point	v;
	// during the first two minutes, done add the periodic error
	// that we would later like to correct using the adaptive optics
	// functionality
	if (age > 120) {
		double	phi = 0.05 * (simtime() - starttime);
		v = Point(_amplitude * cos(3 * phi),
			0.9 * _amplitude * sin(4 * phi));
	}
	v = v + get() * pixels_fullrange;
	return Rotation(alpha)(v);
}

} // namespace simulator
} // namespace camera
} // namespace astro
