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
Device::Device(const std::string& devname, DeviceName::device_type type) noexcept
	: Properties(devname), _name(devname), _controllingName(devname) {
	_name.type(type);
}

/**
 * \brief Construct a device from a device name
 */
Device::Device(const DeviceName& devname, DeviceName::device_type type) noexcept
	: Properties(devname), _name(devname), _controllingName(_name) {
	_name.type(type);
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
		std::string	msg = stringprintf("%s has no parameter named '%s'",
			_name.toString().c_str(), name.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return result;
}

/**
 * \brief Find a modifiable parameter
 */
Device::parametermap_t::iterator	Device::findParameter(
						const std::string& name) {
	parametermap_t::iterator	result = _parameters.find(name);
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
void	Device::parameter(const std::string& name, float value) {
	findParameter(name)->second.set_float(value);
}

/**
 * \brief set a parameter to a string
 */
void	Device::parameter(const std::string& name, const std::string& value) {
	findParameter(name)->second.set_string(value);
}

/**
 * \brief get the float value of a parameter
 */
float	Device::parameterValueFloat(const std::string& name) const {
	return findParameter(name)->second.get_float();
}

/**
 * \brief get the string value of a parameter
 */
std::string	Device::parameterValueString(const std::string& name) const {
	return findParameter(name)->second.get_string();
}

void	Device::add(ParameterDescription parameter) {
	_parameters.insert(std::make_pair(parameter.name(), parameter));
}

/**
 * \brief get the user friendly name of a device
 */
std::string	Device::userFriendlyName() const {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"%s does not override userFriendlyName()",
		astro::demangle_string(*this).c_str());
	return name().toString();
}

/**
 * \brief Get the controlling device name
 */
const std::string	Device::controllingName() {
	std::unique_lock<std::recursive_mutex>	_lock(_controlling_mutex);
	return _controllingName;
}

/**
 * \brief set the controlling device name
 *
 * \param dn	name of the controlling device
 */
void	Device::controllingName(const std::string& dn) {
	std::unique_lock<std::recursive_mutex>	_lock(_controlling_mutex);
	_controllingName = dn;
}

/**
 * \brief retrieve the controlling state
 */
Device::controlState_t	Device::controllingState() {
	std::unique_lock<std::recursive_mutex>	_lock(_controlling_mutex);
	return _controllingState;
}

/**
 * \brief Set the controlling state
 *
 * \param cs	the controlling state to set
 */
void Device::controllingState(controlState_t cs) {
	std::unique_lock<std::recursive_mutex>	_lock(_controlling_mutex);
	_controllingState = cs;
}

/**
 * \brief Start controlling a device
 *
 * \param dn	the controlling device name
 * \param cs	the new controlling state
 */
void	Device::controlling(const std::string& dn, controlState_t cs) {
	std::unique_lock<std::recursive_mutex>	_lock(_controlling_mutex);
	controllingName(dn);
	controllingState(cs);
}

/**
 * \brief Release any controlling devices for this device
 */
void	Device::releaseControlling() {
	std::unique_lock<std::recursive_mutex>	_lock(_controlling_mutex);
	controllingName(_name);
	controllingState(ControllingNone);
}

/**
 * \brief Convenience function to find out whether a device is controlled
 */
bool	Device::isControlled() {
	std::unique_lock<std::recursive_mutex>	_lock(_controlling_mutex);
	return (_controllingState != ControllingNone);
}

} // namespace device
} // namespace astro
