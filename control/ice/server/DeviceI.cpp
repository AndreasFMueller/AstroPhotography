/*
 * DeviceI.cpp -- device 
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <DeviceI.h>
#include <IceConversions.h>

namespace snowstar {

DeviceI::DeviceI(astro::device::Device& device) : _device(device) {
}

DeviceI::~DeviceI() {
}

std::string	DeviceI::getName(const Ice::Current& /* current */) {
	return _device.name().toString();
}

std::vector<std::string>	DeviceI::parameterNames(
					const Ice::Current& /* current */) {
	std::vector<std::string>	result;
	std::list<std::string>	pn = _device.parameterNames();
	copy(pn.begin(), pn.end(), std::back_inserter(result));
	return result;
}

bool	DeviceI::hasParameter(const std::string& name,
		const Ice::Current& /* current */) {
	return _device.hasParameter(name);
}

ParameterDescription	DeviceI::parameter(const std::string& name,
		const Ice::Current& /* current */) {
	return convert(_device.parameter(name));
}

void	DeviceI::setParameterFloat(const std::string& name,
		double value, const Ice::Current& /* current */) {
	_device.parameter(name, (float)value);
}

void	DeviceI::setParameterString(const std::string& name,
		const std::string& value, const Ice::Current& /* current */) {
	_device.parameter(name, value);
}

float	DeviceI::parameterValueFloat(const std::string& name,
		const Ice::Current& /* current */) {
	return _device.parameterValueFloat(name);
}

std::string	DeviceI::parameterValueString(const std::string& name,
		const Ice::Current& /* current */) {
	return _device.parameterValueString(name);
}



} // namespace snowstar


