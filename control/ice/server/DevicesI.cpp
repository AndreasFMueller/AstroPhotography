/*
 * DevicesI.cpp -- Device access servant implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <DevicesI.h>
#include <AstroLoader.h>
#include <ProxyCreator.h>
#include <IceConversions.h>
#include <AstroDebug.h>
#include <NameConverter.h>
#include <AstroDebug.h>

namespace snowstar {

DevicesI::DevicesI(astro::module::Devices& devices)
	: _devices(devices) {
}

DevicesI::~DevicesI() {
}

DeviceNameList DevicesI::getDevicelist(devicetype type,
			const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		astro::module::Devices::devicelist	devicelist
			= _devices.getDevicelist(convert(type));
		return convert(devicelist);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf("cannot get devices: %s",
			x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", x.what());
		throw notfound;
	}
}

AdaptiveOpticsPrx	DevicesI::getAdaptiveOptics(const std::string& name,
					const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve AO %s", name.c_str());
	try {
		return createProxy<AdaptiveOpticsPrx>(name, current);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf(
			"cannot get AdaptiveOptics proxy: %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", x.what());
		throw notfound;
	}
}

CameraPrx	DevicesI::getCamera(const std::string& name,
					const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve Camera %s", name.c_str());
	try {
		return createProxy<CameraPrx>(name, current);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf(
			"cannot get Camera proxy: %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", x.what());
		throw notfound;
	}
}

CcdPrx		DevicesI::getCcd(const std::string& name,
					const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve CCD %s", name.c_str());
	try {
		return createProxy<CcdPrx>(name, current);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf(
			"cannot get Ccd proxy: %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", x.what());
		throw notfound;
	}
}

GuidePortPrx	DevicesI::getGuidePort(const std::string& name,
					const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve GuidePort %s", name.c_str());
	try {
		return createProxy<GuidePortPrx>(name, current);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf(
			"cannot get GuidePort proxy: %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", x.what());
		throw notfound;
	}
}

FilterWheelPrx	DevicesI::getFilterWheel(const std::string& name,
					const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve FilterWheel %s", name.c_str());
	try {
		return createProxy<FilterWheelPrx>(name, current);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf(
			"cannot get FilterWheel proxy: %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", x.what());
		throw notfound;
	}
}

CoolerPrx	DevicesI::getCooler(const std::string& name,
					const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve Cooler %s", name.c_str());
	try {
		return createProxy<CoolerPrx>(name, current);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf(
			"cannot get Cooler proxy: %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", x.what());
		throw notfound;
	}
}

FocuserPrx	DevicesI::getFocuser(const std::string& name,
					const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve Focuser %s", name.c_str());
	try {
		return createProxy<FocuserPrx>(name, current);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf(
			"cannot get Focuser proxy: %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", x.what());
		throw notfound;
	}
}

MountPrx	DevicesI::getMount(const std::string& name,
					const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve Mount %s", name.c_str());
	try {
		return createProxy<MountPrx>(name, current);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf(
			"cannot get Mount proxy: %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", x.what());
		throw notfound;
	}
}

} // namespace snowstar
