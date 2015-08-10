/*
 * DeviceLocatorI.h -- servant interface for the device locator
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DeviceLocatorI_h
#define _DeviceLocatorI_h

#include <device.h>
#include <AstroLocator.h>

namespace snowstar {

class DeviceLocatorI : virtual public DeviceLocator {
	astro::device::DeviceLocatorPtr	_locator;
public:
	DeviceLocatorI(astro::device::DeviceLocatorPtr locator);
	virtual ~DeviceLocatorI();
	virtual std::string	getName(const Ice::Current& current);
	virtual std::string	getVersion(const Ice::Current& current);
	virtual DeviceNameList	getDevicelist(devicetype type,
					const Ice::Current& current);
	
	virtual AdaptiveOpticsPrx	getAdaptiveOptics(const std::string& name,
					const Ice::Current& current);
	virtual CameraPrx	getCamera(const std::string& name,
					const Ice::Current& current);
	virtual CcdPrx		getCcd(const std::string& name,
					const Ice::Current& current);
	virtual GuiderPortPrx	getGuiderPort(const std::string& name,
					const Ice::Current& current);
	virtual FilterWheelPrx	getFilterWheel(const std::string& name,
					const Ice::Current& current);
	virtual CoolerPrx	getCooler(const std::string& name,
					const Ice::Current& current);
	virtual FocuserPrx	getFocuser(const std::string& name,
					const Ice::Current& current);
	virtual MountPrx	getMount(const std::string& name,
					const Ice::Current& current);
};

} // namespace snowstar

#endif /* _DeviceLocatorI_h */
