/*
 * GuiderName.cpp -- implementation of the GuiderName base class
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <sstream>

namespace astro {
namespace guiding {

/**
 * \brief Parse a guider name
 */
void	GuiderName::parse(const std::string& n) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "name = %s", n.c_str());
	// split the name at |, you should get 4 components
	std::vector<std::string>	components;
	split(n, "|", components);
	if (4 != components.size()) {
		std::string	msg = stringprintf("wrong number of components,"
			" %d instead of 4 in name '%s'", components.size(),
			n.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_instrument = components[0];
	if (components[1].size() > 0) {
		_ccdIndex = std::stoi(components[1]);
	} else {
		_ccdIndex = -1;
	}
	if (components[2].size() > 0) {
		_guiderportIndex = std::stoi(components[2]);
	} else {
		_guiderportIndex = -1;
	}
	if (components[3].size() > 0) {
		_adaptiveopticsIndex = std::stoi(components[3]);
	} else {
		_adaptiveopticsIndex = -1;
	}
	if (_ccdIndex < 0) {
		std::string	msg = stringprintf("cannot parse ccdIndex from "
			"%s", n.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	// verify work: build a name and verify that it coincides with
	// the name we originally got
	std::string	n2 = buildname();
	if (n != n2) {
		std::string	msg = stringprintf("name parsing fails: "
			"%s != %s", n.c_str(), n2.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}

/**
 * \brief Build a name from instrument name and device indices
 */
std::string	GuiderName::buildname() const {
	std::stringstream	str;
	str << _instrument << "|" << _ccdIndex << "|";
	if (_guiderportIndex >= 0) {
		str << _guiderportIndex;
	}
	str << "|";
	if (_adaptiveopticsIndex >= 0) {
		str << _adaptiveopticsIndex;
	}
	return str.str();
}

/**
 * \brief Construct a guider name
 *
 * This constructor unifies two constructors: if there is only a name specified,
 * then it is assumed that this is a guider name, and the device indices
 * are parsed from that name. If there are indices specified, indicated
 * by the fact that ccdIndex is not negative, then the argument n ist taken
 * as the instrument name, 
 */
GuiderName::GuiderName(const std::string& n, int ccdIndex,
	int guiderportIndex, int adaptiveopticsIndex) {
	if (ccdIndex < 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "GuiderName from name, %s",
			n.c_str());
		parse(n);
		_name = buildname();
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Guider: %s|%d|%d|%d",
		n.c_str(), ccdIndex, guiderportIndex, adaptiveopticsIndex);
	_instrument = n;
	_ccdIndex = ccdIndex;
	_guiderportIndex = guiderportIndex;
	_adaptiveopticsIndex = adaptiveopticsIndex;
	_name = buildname();
}

/**
 * \brief Copy constructor
 */
GuiderName::GuiderName(const GuiderName& other) {
	_name = other._name;
	_instrument = other._instrument;
	_ccdIndex = other._ccdIndex;
	_guiderportIndex = other._guiderportIndex;
	_adaptiveopticsIndex = other._adaptiveopticsIndex;
}

/**
 * \brief update the guider name
 */
void	GuiderName::name(const std::string& n) {
	_name = n;
	parse(n);
}

/**
 * \brief Update the instrument name
 */
void	GuiderName::instrument(const std::string& i) {
	_instrument = i;
	_name = buildname();
}

/**
 * \brief update the ccd index
 */
void	GuiderName::ccdIndex(int c) {
	if (c < 0) {
		std::string	msg = stringprintf("bad ccdIndex %d", c);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_ccdIndex = c;
	_name = buildname();
}

/**
 * \brief update the guiderport index
 */
void	GuiderName::guiderportIndex(int g) {
	if (g < -1) {
		std::string	msg = stringprintf("bad guiderportIndex %d", g);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_guiderportIndex = g;
	_name = buildname();
}

/**
 * \brief update the adaptive optics index
 */
void	GuiderName::adaptiveopticsIndex(int a) {
	if (a < -1) {
		std::string	msg = stringprintf("bad aoIndex %d", a);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_adaptiveopticsIndex = a;
	_name = buildname();
}

/**
 * \brief Get control Device name for guider port control device
 */
ControlDeviceNamePtr	GuiderName::guiderPortDeviceName() {
	if (!hasGuiderPort()) {
		std::string	msg = stringprintf("%s has no guiderport",
			_name.c_str());
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
			_name.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	ControlDeviceName	*result = new ControlDeviceName(*this, AO);
	return ControlDeviceNamePtr(result);
}

} // namespace guiding
} // namespace astro
