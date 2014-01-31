/*
 * DeviceLocatorI.cpp -- Device Locator implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <DeviceLocatorI.h>

namespace snowstar {

std::string	DeviceLocatorI::getName(const Ice::Current& current) {
	return _locator->getName();
}

std::string	DeviceLocatorI::getVersion(const Ice::Current& current) {
	return _locator->getVersion();
}

DeviceNameList	DeviceLocatorI::getDevicelist(devicetype type,
			const Ice::Current& current) {
	astro::DeviceName::device_type	t;
	switch (type) {
	case DevAO:
		t = astro::DeviceName::AdaptiveOptics;
		break;
	case DevCAMERA:
		t = astro::DeviceName::Camera;
		break;
	case DevCCD:
		t = astro::DeviceName::Ccd;
		break;
	case DevCOOLER:
		t = astro::DeviceName::Cooler;
		break;
	case DevFILTERWHEEL:
		t = astro::DeviceName::Filterwheel;
		break;
	case DevFOCUSER:
		t = astro::DeviceName::Focuser;
		break;
	case DevGUIDERPORT:
		t = astro::DeviceName::Guiderport;
		break;
	case DevMODULE:
		t = astro::DeviceName::Module;
		break;
	};
	return _locator->getDevicelist(t);
}

CameraPrx	DeviceLocatorI::getCamera(const std::string& name,
			const Ice::Current& current) {
	return NULL;
}

CcdPrx	DeviceLocatorI::getCcd(const std::string& name,
			const Ice::Current& current) {
	return NULL;
}

GuiderPortPrx	DeviceLocatorI::getGuiderPort(const std::string& name,
			const Ice::Current& current) {
	return NULL;
}

FilterWheelPrx	DeviceLocatorI::getFilterWheel(const std::string& name,
			const Ice::Current& current) {
	return NULL;
}

CoolerPrx	DeviceLocatorI::getCooler(const std::string& name,
			const Ice::Current& current) {
	return NULL;
}

FocuserPrx	DeviceLocatorI::getFocuser(const std::string& name,
			const Ice::Current& current) {
	return NULL;
}

} // namespace snowstar
