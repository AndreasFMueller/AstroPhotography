/*
 * AdaptiveOptics.cpp -- implementation of adaptive optics base interface
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroExceptions.h>
#include <AstroDebug.h>
#include <AstroFormat.h>

namespace astro {
namespace camera {

DeviceName::device_type	AdaptiveOptics::devicetype = DeviceName::AdaptiveOptics;

AdaptiveOptics::AdaptiveOptics(const DeviceName& name)
	: Device(name, DeviceName::AdaptiveOptics), _hasguideport(false) {
}

AdaptiveOptics::AdaptiveOptics(const std::string& name)
	: Device(name, DeviceName::AdaptiveOptics), _hasguideport(false) {
}

AdaptiveOptics::~AdaptiveOptics() {
}

void	AdaptiveOptics::set(const Point& position) {
	if ((position.x() < -1) || (position.x() > 1)) {
		throw std::range_error("x position out of range");
	}
	if ((position.y() < -1) || (position.y() > 1)) {
		throw std::range_error("y position out of range");
	}
	try {
		this->set0(position);
		currentposition = position;
		callback(currentposition);
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "error during positioning: %s",
			x.what());
		throw x;
	}
}

void	AdaptiveOptics::set0(const Point& /* position */) {
	throw NotImplemented("get/set must be implement in driver class");
}

void	AdaptiveOptics::center() {
	this->set0(Point(0, 0));
}

GuidePortPtr	AdaptiveOptics::getGuidePort() {
	if (!hasGuidePort()) {
		std::string	msg("AO unit has no guide port");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return this->getGuidePort0();
}

GuidePortPtr	AdaptiveOptics::getGuidePort0() {
	std::string	msg("guide port not implemented, have you called "
		"hasGuiderPort()?");
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw NotImplemented(msg);
}

void	AdaptiveOptics::addCallback(callback::CallbackPtr callback) {
	_callback.insert(callback);
}

void	AdaptiveOptics::removeCallback(callback::CallbackPtr callback) {
	auto	i = _callback.find(callback);
	if (i != _callback.end()) {
		_callback.erase(i);
	}
}

void	AdaptiveOptics::callback(const Point& point) {
	callback::CallbackDataPtr	cb(new callback::PointCallbackData(point));
	_callback(cb);
}

} // namespace camera
} // namespace astro
