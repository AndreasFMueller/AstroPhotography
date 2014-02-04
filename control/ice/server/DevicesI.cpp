/*
 * DevicesI.cpp -- Device access servant implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <DevicesI.h>
#include <AstroLoader.h>
#include <ProxyCreator.h>

namespace snowstar {

DevicesI::DevicesI(astro::module::Devices& devices)
	: _devices(devices) {
}

DevicesI::~DevicesI() {
}

DeviceNameList DevicesI::getDevicelist(devicetype type,
			const Ice::Current& current) {
	astro::module::Devices::devicelist	devicelist
		= _devices.getDevicelist(convert(type));
	return convert(devicelist);
}

CameraPrx	DevicesI::getCamera(const std::string& name,
					const Ice::Current& current) {
	return createProxy<CameraPrx>(name, current);
}

CcdPrx		DevicesI::getCcd(const std::string& name,
					const Ice::Current& current) {
	return createProxy<CcdPrx>(name, current);
}

GuiderPortPrx	DevicesI::getGuiderPort(const std::string& name,
					const Ice::Current& current) {
	return createProxy<GuiderPortPrx>(name, current);
}

FilterWheelPrx	DevicesI::getFilterWheel(const std::string& name,
					const Ice::Current& current) {
	return createProxy<FilterWheelPrx>(name, current);
}

CoolerPrx	DevicesI::getCooler(const std::string& name,
					const Ice::Current& current) {
	return createProxy<CoolerPrx>(name, current);
}

FocuserPrx	DevicesI::getFocuser(const std::string& name,
					const Ice::Current& current) {
	return createProxy<FocuserPrx>(name, current);
}

} // namespace snowstar
