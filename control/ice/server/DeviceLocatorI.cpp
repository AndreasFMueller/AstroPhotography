/*
 * DeviceLocatorI.cpp -- servant for device locator implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <DeviceLocatorI.h>
#include <DevicesI.h>
#include <ProxyCreator.h>
#include <IceConversions.h>

namespace snowstar {

DeviceLocatorI::DeviceLocatorI(astro::device::DeviceLocatorPtr locator)
	: _locator(locator) {
}

DeviceLocatorI::~DeviceLocatorI() {
}

DeviceNameList	DeviceLocatorI::getDevicelist(devicetype type,
	const Ice::Current& current) {
	CallStatistics::count(current);
	return _locator->getDevicelist(convert(type));
}

std::string	DeviceLocatorI::getName(const Ice::Current& current) {
	CallStatistics::count(current);
	return _locator->getName();
}

std::string	DeviceLocatorI::getVersion(const Ice::Current& current) {
	CallStatistics::count(current);
	return _locator->getVersion();
}

AdaptiveOpticsPrx	DeviceLocatorI::getAdaptiveOptics(const std::string& name,
			const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		return createProxy<AdaptiveOpticsPrx>(name, current);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf(
			"AdaptiveOptics %s not found: %s",
			name.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", notfound.cause.c_str());
		throw notfound;
	}
}

CameraPrx	DeviceLocatorI::getCamera(const std::string& name,
			const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		return createProxy<CameraPrx>(name, current);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf("Camera %s not found: %s",
			name.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", notfound.cause.c_str());
		throw notfound;
	}
}

CcdPrx          DeviceLocatorI::getCcd(const std::string& name,
                                        const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		return createProxy<CcdPrx>(name, current);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf("Ccd %s not found: %s",
			name.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", notfound.cause.c_str());
		throw notfound;
	}
}

GuidePortPrx   DeviceLocatorI::getGuidePort(const std::string& name,
					const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		return createProxy<GuidePortPrx>(name, current);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf("GuidePort %s not found: %s",
			name.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", notfound.cause.c_str());
		throw notfound;
	}
}

FilterWheelPrx  DeviceLocatorI::getFilterWheel(const std::string& name,
					const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		return createProxy<FilterWheelPrx>(name, current);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf(
			"FilterWheel %s not found: %s",
			name.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", notfound.cause.c_str());
		throw notfound;
	}
}

CoolerPrx       DeviceLocatorI::getCooler(const std::string& name,
					const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		return createProxy<CoolerPrx>(name, current);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf("Cooler %s not found: %s",
			name.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", notfound.cause.c_str());
		throw notfound;
	}
}

FocuserPrx      DeviceLocatorI::getFocuser(const std::string& name,
					const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		return createProxy<FocuserPrx>(name, current);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf("Focuser %s not found: %s",
			name.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", notfound.cause.c_str());
		throw notfound;
	}
}

MountPrx      DeviceLocatorI::getMount(const std::string& name,
					const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		return createProxy<MountPrx>(name, current);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf("Mount %s not found: %s",
			name.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", notfound.cause.c_str());
		throw notfound;
	}
}


} // namespace snowstar
