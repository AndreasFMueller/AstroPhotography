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
			const Ice::Current& /* current */) {
	astro::module::Devices::devicelist	devicelist
		= _devices.getDevicelist(convert(type));
	return convert(devicelist);
}

AdaptiveOpticsPrx	DevicesI::getAdaptiveOptics(const std::string& name,
					const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve AO %s", name.c_str());
	return createProxy<AdaptiveOpticsPrx>(name, current);
}

CameraPrx	DevicesI::getCamera(const std::string& name,
					const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve Camera %s", name.c_str());
	return createProxy<CameraPrx>(name, current);
}

CcdPrx		DevicesI::getCcd(const std::string& name,
					const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve CCD %s", name.c_str());
	return createProxy<CcdPrx>(name, current);
}

GuiderPortPrx	DevicesI::getGuiderPort(const std::string& name,
					const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve GuiderPort %s", name.c_str());
	return createProxy<GuiderPortPrx>(name, current);
}

FilterWheelPrx	DevicesI::getFilterWheel(const std::string& name,
					const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve FilterWheel %s", name.c_str());
	return createProxy<FilterWheelPrx>(name, current);
}

CoolerPrx	DevicesI::getCooler(const std::string& name,
					const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve Cooler %s", name.c_str());
	return createProxy<CoolerPrx>(name, current);
}

FocuserPrx	DevicesI::getFocuser(const std::string& name,
					const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve Focuser %s", name.c_str());
	return createProxy<FocuserPrx>(name, current);
}

MountPrx	DevicesI::getMount(const std::string& name,
					const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve Mount %s", name.c_str());
	return createProxy<MountPrx>(name, current);
}

} // namespace snowstar
