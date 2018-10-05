/*
 * SimMount.cpp -- simulated mount implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimMount.h>

using namespace astro::device;

namespace astro {
namespace camera {
namespace simulator {

/**
 * \brief Construct a simulated mount
 *
 * \param locator	common simulated locator
 */
SimMount::SimMount(SimLocator& locator) 
	 : Mount(DeviceName("mount:simulator/mount")), _locator(locator) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing simulated mount");
	_when = 0;
	_direction = _target;
}

const static double _movetime = 10;

/**
 * \param update the state variables
 */
void	SimMount::updateState() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	if (_when > 0) {
		double	now = Timer::gettime();
		if (now > _when) {
			_when = 0;
			_direction = _target;
		}
	}
}

/**
 * \brief Determine the state of the mount
 */
Mount::state_type	SimMount::state() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	updateState();
	if (_when > 0) {
		return Mount::GOTO;
	}
	return Mount::TRACKING;
}

/**
 * \brief Compute the direction the mount is currently pointing
 */
RaDec	SimMount::direction() {
	updateState();
	if (_when <= 0) {
		return _direction;
	}
	// perform interpolation between _direction and target
	double	_now = Timer::gettime();
	double	t = (_when - _now) / _movetime;
	return _target * (1 - t) + _direction * t;
}

/**
 * \brief Get the direction into which the mount is pointing
 */
RaDec	SimMount::getRaDec() {
	return direction();
}

/**
 * \brief Get the azimuth and altitude 
 *
 * This method always throws an exception to indicate that the simulated
 * mount does not know about azimuth and altitude
 */
AzmAlt	SimMount::getAzmAlt() {
	debug(LOG_ERR, DEBUG_LOG, 0, "cannot get AzmAlt");
	throw std::runtime_error("cannot get AzmAlt");
}

/**
 * \brief Move to a new position in right ascension and declination
 *
 * We should make this dynamic so that the simulated mount takes some
 * time to move to the new position
 *
 * \param radec		new coordinates
 */
void	SimMount::Goto(const RaDec& radec) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	_direction = direction();
	_when = Timer::gettime() + _movetime;
	_target = radec;
}

/**
 * \brief Move to a new position in azimuth and altitude
 *
 * This method always throws an exception to indicate that the simulated
 * mount does not understand altitude and azimuth
 */
void	SimMount::Goto(const AzmAlt& /* azmalt */) {
	debug(LOG_ERR, DEBUG_LOG, 0, "cannot get AzmAlt");
	throw std::runtime_error("cannot goto AzmAlt");
}

/**
 * \brief Cancel movement
 *
 * This method is not implemented
 */
void	SimMount::cancel() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	_when = 0;
	_target = _direction;
}

#if 0
/**
 * \brief set a parameter by name
 *
 * \param name		name of the parameter
 * \param angle		angle in degrees of the parameter
 */
void	SimMount::parameter(const std::string& name, float angle) {
	if (name == std::string("longitude")) {
		_position.longitude().degrees(angle);
		return;
	}
	if (name == std::string("latitude")) {
		_position.latitude().degrees(angle);
		return;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "no parameter %s, %f",
		name.c_str(), angle);
	throw std::logic_error("no such parameter");
}

/**
 * \brief get the value of a parameter
 *
 * \param name		name of the parameter
 */
float	SimMount::parameterValueFloat(const std::string& name) const {
	if (name == std::string("longitude")) {
		return _position.longitude().degrees();
	}
	if (name == std::string("latitude")) {
		return _position.latitude().degrees();
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "no parameter named '%s'", name.c_str());
	throw std::logic_error("no such parameter");
}
#endif

} // namespace simulator
} // namespace camera
} // namespace astro
