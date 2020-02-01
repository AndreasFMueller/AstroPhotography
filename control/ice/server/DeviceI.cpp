/*
 * DeviceI.cpp -- device 
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <DeviceI.h>
#include <IceConversions.h>

namespace snowstar {

DeviceI::DeviceI(astro::device::Device& device) : _device(device) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing device %s",
		_device.name().toString().c_str());
	std::list<std::string>	pn = _device.parameterNames();
	std::list<std::string>::const_iterator	s;
	for (s = pn.begin(); s != pn.end(); s++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s parameter: %s",
			_device.name().toString().c_str(), s->c_str());
	}
}

DeviceI::~DeviceI() {
}

std::string	DeviceI::getName(const Ice::Current& current) {
	CallStatistics::count(current);
	return _device.name().toString();
}

std::vector<std::string>	DeviceI::parameterNames(
					const Ice::Current& current) {
	CallStatistics::count(current);
	std::vector<std::string>	result;
	std::list<std::string>	pn = _device.parameterNames();
	copy(pn.begin(), pn.end(), std::back_inserter(result));
	return result;
}

bool	DeviceI::hasParameter(const std::string& name,
		const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "does device parameter '%s' exist?",
		name.c_str());
	return _device.hasParameter(name);
}

ParameterDescription	DeviceI::parameter(const std::string& name,
		const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "querying device parameter '%s'",
		name.c_str());
	return convert(_device.parameter(name));
}

void	DeviceI::setParameterFloat(const std::string& name,
		double value, const Ice::Current& current) {
	CallStatistics::count(current);
	_device.parameter(name, (float)value);
}

void	DeviceI::setParameterString(const std::string& name,
		const std::string& value, const Ice::Current& current) {
	CallStatistics::count(current);
	_device.parameter(name, value);
}

float	DeviceI::parameterValueFloat(const std::string& name,
		const Ice::Current& current) {
	CallStatistics::count(current);
	return _device.parameterValueFloat(name);
}

std::string	DeviceI::parameterValueString(const std::string& name,
		const Ice::Current& current) {
	CallStatistics::count(current);
	return _device.parameterValueString(name);
}



} // namespace snowstar


