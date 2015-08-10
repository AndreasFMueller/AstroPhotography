/*
 * Device.cpp -- Device property interface
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDevice.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <stdexcept>

namespace astro {
namespace device {

/**
 * \brief Construct a device
 */
Device::Device(const std::string& devname, DeviceName::device_type type)
	: Properties(devname), _name(devname) {
	if (name().type() != type) {
		throw std::runtime_error("name type does not match");
	}
}

/**
 * \brief Construct a device from a device name
 */
Device::Device(const DeviceName& devname, DeviceName::device_type type)
	: Properties(devname), _name(devname) {
	if (name().type() != type) {
		throw std::runtime_error("name type does not match");
	}
}

/**
 * \brief Destructor
 */
Device::~Device() {
}

/**
 * \brief Get a list of parameters
 */
std::list<std::string>	Device::parameterNames() const {
	std::list<std::string>	result;
	parametermap_t::const_iterator	i;
	for (i = _parameters.begin(); i != _parameters.end(); i++) {
		result.push_back(i->first);
	}
	return result;
}

/**
 * \brief find a parameter iterator
 *
 * This is a common code in many methods below, so it makes sense to
 * consolidate it into a single function
 */
Device::parametermap_t::const_iterator	Device::findParameter(
						const std::string& name) const {
	parametermap_t::const_iterator	result = _parameters.find(name);
	if (result == _parameters.end()) {
		std::string	msg = stringprintf("no parameter named '%s'",
			name.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return result;
}

/**
 * \brief Find out whether we have a parameter of this name
 */
bool	Device::hasParameter(const std::string& name) const {
	return (_parameters.find(name) != _parameters.end());
}

/**
 * \brief get the parameter description
 */
ParameterDescription	Device::parameter(const std::string& name) const {
	parametermap_t::const_iterator	result = findParameter(name);
	return result->second;
}

/**
 * \brief set the parameter value
 *
 * The device base class does not have any parameters, so it only does the
 * test 
 */
void	Device::parameter(const std::string& name, float /* value */) {
	findParameter(name);
}

/**
 * \brief set a parameter to a string
 */
void	Device::parameter(const std::string& name, const std::string& /* value */) {
	findParameter(name);
}

/**
 * \brief get the float value of a parameter
 */
float	Device::parameterValueFloat(const std::string& /* name */) const {
	throw std::logic_error("Device class has no parameters");
}

/**
 * \brief get the string value of a parameter
 */
std::string	Device::parameterValueString(const std::string& /* name */) const {
	throw std::logic_error("Device class has no parameters");
}

void	Device::add(ParameterDescription parameter) {
	_parameters.insert(std::make_pair(parameter.name(), parameter));
}

} // namespace device
} // namespace astro
