/*
 * AdaptiveOptics.cpp -- implementation of adaptive optics base interface
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroExceptions.h>

namespace astro {
namespace camera {

DeviceName::device_type	AdaptiveOptics::devicetype = DeviceName::AdaptiveOptics;

AdaptiveOptics::AdaptiveOptics(const DeviceName& name) : Device(name) {
}

AdaptiveOptics::AdaptiveOptics(const std::string& name) : Device(name) {
}

AdaptiveOptics::~AdaptiveOptics() {
}

void	AdaptiveOptics::set(const Point& position) {
	if ((position.x() < -1) || (position.x() > 1)) {
		throw std::range_error("x position out of range");
	}
	if ((position.y() < -1) || (position.y() > 1)) {
		throw std::range_error("y position out of range");
	}
	try {
		this->set0(position);
		currentposition = position;
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "error during positioning: %s",
			x.what());
		throw x;
	}
}

void	AdaptiveOptics::set0(const Point& position) {
	throw NotImplemented("get/set must be implement in driver class");
}

void	AdaptiveOptics::center() {
	this->set0(Point(0, 0));
}

GuiderPortPtr	AdaptiveOptics::getGuiderPort() {
	if (!hasGuiderPort()) {
		throw std::runtime_error("AO unit has no guider port");
	}
	return this->getGuiderPort0();
}

GuiderPortPtr	AdaptiveOptics::getGuiderPort0() {
	throw NotImplemented("guider port not implemented");
}

} // namespace camera
} // namespace astro
