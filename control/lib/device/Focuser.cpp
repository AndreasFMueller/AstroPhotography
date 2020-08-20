/*
 * Focuser.cpp -- Focuser base class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroExceptions.h>
#include <AstroDebug.h>
#include <AstroIO.h>
#include <includes.h>

using namespace astro::device;

namespace astro {
namespace camera {

DeviceName::device_type	Focuser::devicetype = DeviceName::Focuser;

Focuser::Focuser(const DeviceName& name) : Device(name, DeviceName::Focuser) {
	_targetposition = 0;
}

Focuser::Focuser(const std::string& name) : Device(name, DeviceName::Focuser) {
	_targetposition = 0;
}

Focuser::~Focuser() {
}

long	Focuser::min() {
	return 0;
}

long	Focuser::max() {
	return std::numeric_limits<unsigned short>::max();
}

long	Focuser::current() {
	throw NotImplemented("base Focuser does not implement current method");
}

long	Focuser::backlash() {
	if (hasProperty("backlash")) {
		return std::stoi(getProperty("backlash"));
	}
	return 0;
}

void	Focuser::set(long value) {
	if (value < this->min()) {
		std::string	msg = stringprintf("%ld too small (< %ld)",
			value, this->min());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}
	if (value > this->max()) {
		std::string	msg = stringprintf("%ld too large (> %ld)",
			value, this->max());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}
	_targetposition = value;
	callback(this->current(), value);
}

/**
 * \brief Position focuser and wait for completion
 *
 * This method calls the set method but then waits until either
 * the position is reached, or the timeout occurs.
 *
 * \param value		new focuser position
 * \param timeout	maximum numer of seconds to wait for the focuser to
 *			reach the new position
 */
bool	Focuser::moveto(long value, unsigned long timeout) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "moving to %lu", value);
	// record current time
	time_t	starttime;
	time(&starttime);
	time_t	timelimit = starttime + timeout;

	// ensure we stay within the limits
	if (value > max()) {
		value = max();
	}
	if (value < min()) {
		value = min();
	}

	// start moving to this position
	set(value);

	// wait until we reach the position
	time_t	now;
	time(&now);
	long	currentposition = current();
	do {
		if (value != currentposition) {
			usleep(100000);
		}
		currentposition = current();
		time(&now);
	} while ((currentposition != value) && (now < timelimit));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "final position is %lu after %d seconds",
		currentposition, now - starttime);

	// report whether we have reached the position
	return (currentposition == value);
}

/**
 * \brief Add focus position to the image metadata
 *
 * \param image		image to add the info to
 */
void	Focuser::addFocusMetadata(ImageBase& image) {
	image.setMetadata(astro::io::FITSKeywords::meta("FOCUSPOS",
		current()));
}

void	Focuser::callback(long position, bool on_target) {
	callback::CallbackDataPtr cb(new FocuserPositionInfoCallbackData(
		FocuserPositionInfo(position, on_target)));
	_callback(cb);
}

void	Focuser::callback(long currentposition, long newposition) {
	callback::CallbackDataPtr cb(new FocuserMovementInfoCallbackData(
		FocuserMovementInfo(currentposition, newposition)));
	_callback(cb);
}

void	Focuser::addCallback(callback::CallbackPtr callback) {
	_callback.insert(callback);
}

void	Focuser::removeCallback(callback::CallbackPtr callback) {
	auto	i = _callback.find(callback);
	if (i != _callback.end()) {
		_callback.erase(i);
	}
}

} // namespace camera
} // namespace astro
