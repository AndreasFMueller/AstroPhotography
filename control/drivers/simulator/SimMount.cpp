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

SimMount::SimMount(SimLocator& locator) 
	 : Mount(DeviceName("mount:simulator/mount")), _locator(locator) {
	ParameterDescription	longitude("longitude", -180, 180);
	add(longitude);
	ParameterDescription	latitude("latitude", -90, 90);
	add(latitude);
	// set the position to altendorf
	_position.longitude().degrees(8.83);
	_position.latitude().degrees(47.19);
}

Mount::state_type	SimMount::state() {
	return Mount::IDLE;
}

RaDec	SimMount::getRaDec() {
	return _direction;
}

AzmAlt	SimMount::getAzmAlt() {
	debug(LOG_ERR, DEBUG_LOG, 0, "cannot get AzmAlt");
	throw std::runtime_error("XXX cannot get AzmAlt");
}

void	SimMount::Goto(const RaDec& radec) {
	_direction = radec;
}

void	SimMount::Goto(const AzmAlt& /* azmalt */) {
	debug(LOG_ERR, DEBUG_LOG, 0, "cannot get AzmAlt");
	throw std::runtime_error("XXX cannot goto AzmAlt");
}

void	SimMount::cancel() {
	// XXX implementation missing
}

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


} // namespace simulator
} // namespace camera
} // namespace astro
