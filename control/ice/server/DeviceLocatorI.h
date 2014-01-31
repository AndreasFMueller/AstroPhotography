/*
 * DeviceLocatorI.h -- DeviceLocator ICE interface
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DeviceLocatorI_h
#define _DeviceLocatorI_h

#include <module.h>
#include <AstroLoader.h>
#include <AstroDevice.h>
#include <AstroLocator.h>

namespace snowstar {

/**
 * \brief Device Locator servant implementation
 */
class DeviceLocatorI : public DeviceLocator {
	astro::device::DeviceLocatorPtr	_locator;
public:
	DeviceLocatorI(astro::device::DeviceLocatorPtr locator)
		: _locator(locator) { }
	virtual ~DeviceLocatorI() { }
	std::string	getName(const Ice::Current& current);
	std::string	getVersion(const Ice::Current& current);
	DeviceNameList	getDevicelist(devicetype type,
		const Ice::Current& current);
	CameraPrx	getCamera(const std::string& name,
		const Ice::Current& current);
	CcdPrx	getCcd(const std::string& name,
		const Ice::Current& current);
	GuiderPortPrx	getGuiderPort(const std::string& name,
		const Ice::Current& current);
	FilterWheelPrx	getFilterWheel(const std::string& name,
		const Ice::Current& current);
	CoolerPrx	getCooler(const std::string& name,
		const Ice::Current& current);
	FocuserPrx	getFocuser(const std::string& name,
		const Ice::Current& current);
};

} // namespace snowstar

#endif /* _DeviceLocatorI_h */
