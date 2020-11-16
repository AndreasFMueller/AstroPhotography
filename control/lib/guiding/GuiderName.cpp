/*
 * GuiderName.cpp -- implementation of the GuiderName base class
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <AstroDiscovery.h>
#include <sstream>

namespace astro {
namespace guiding {

/**
 * \brief Construct a guider name
 *
 * This constructor unifies two constructors: if there is only a name specified,
 * then it is assumed that this is a guider name, and the device indices
 * are parsed from that name. If there are indices specified, indicated
 * by the fact that ccdIndex is not negative, then the argument n ist taken
 * as the instrument name, 
 */
GuiderName::GuiderName(const std::string& n) : _instrument(n) {
	if (!discover::InstrumentBackend::has(_instrument)) {
		std::string	msg = stringprintf("no instrument '%s'",
			_instrument.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", _instrument.c_str()),
		throw std::runtime_error(_instrument);
	}
}

/**
 * \brief Copy constructor
 */
GuiderName::GuiderName(const GuiderName& other) {
	_instrument = other._instrument;
}

GuiderName&	GuiderName::operator=(const GuiderName& other) {
	_instrument = other.instrument();
	return *this;
}

/**
 * \brief Does this guider have a guideport
 */
bool	GuiderName::hasGuidePort() const {
	return discover::InstrumentBackend::get(_instrument)->hasGuidePort();
}

/**
 * \brief Does this guider have an adaptive optics unit
 */
bool	GuiderName::hasAdaptiveOptics() const {
	return discover::InstrumentBackend::get(_instrument)->hasAdaptiveOptics();
}

/**
 * \brief Get control Device name for guider port control device
 */
ControlDeviceNamePtr	GuiderName::guidePortDeviceName() {
	if (!hasGuidePort()) {
		std::string	msg = stringprintf("%s has no guideport",
			_instrument.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	ControlDeviceName	*result = new ControlDeviceName(*this, GP);
	return ControlDeviceNamePtr(result);
}

/**
 * \brief Get control Device name for adaptive optics control device
 */
ControlDeviceNamePtr	GuiderName::adaptiveOpticsDeviceName() {
	if (!hasAdaptiveOptics()) {
		std::string	msg = stringprintf("%s has no AO",
			_instrument.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	ControlDeviceName	*result = new ControlDeviceName(*this, AO);
	return ControlDeviceNamePtr(result);
}

} // namespace guiding
} // namespace astro
