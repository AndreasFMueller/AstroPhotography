/*
 * DeviceLocatorI.cpp -- servant for device locator implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <DeviceLocatorI.h>
#include <DevicesI.h>
#include <ProxyCreator.h>

namespace snowstar {

DeviceLocatorI::DeviceLocatorI(astro::device::DeviceLocatorPtr locator)
	: _locator(locator) {
}

DeviceLocatorI::~DeviceLocatorI() {
}

DeviceNameList	DeviceLocatorI::getDevicelist(devicetype type,
	const Ice::Current& /* current */) {
	return _locator->getDevicelist(DevicesI::convert(type));
}

std::string	DeviceLocatorI::getName(const Ice::Current& /* current */) {
	return _locator->getName();
}

std::string	DeviceLocatorI::getVersion(const Ice::Current& /* current */) {
	return _locator->getVersion();
}

CameraPrx	DeviceLocatorI::getCamera(const std::string& name,
			const Ice::Current& current) {
	return createProxy<CameraPrx>(name, current);
}

CcdPrx          DeviceLocatorI::getCcd(const std::string& name,
                                        const Ice::Current& current) {
	return createProxy<CcdPrx>(name, current);
}

GuiderPortPrx   DeviceLocatorI::getGuiderPort(const std::string& name,
					const Ice::Current& current) {
	return createProxy<GuiderPortPrx>(name, current);
}

FilterWheelPrx  DeviceLocatorI::getFilterWheel(const std::string& name,
					const Ice::Current& current) {
	return createProxy<FilterWheelPrx>(name, current);
}

CoolerPrx       DeviceLocatorI::getCooler(const std::string& name,
					const Ice::Current& current) {
	return createProxy<CoolerPrx>(name, current);
}

FocuserPrx      DeviceLocatorI::getFocuser(const std::string& name,
					const Ice::Current& current) {
	return createProxy<FocuserPrx>(name, current);
}


} // namespace snowstar
