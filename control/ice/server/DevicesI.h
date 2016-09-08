/*
 * DevicesI.h -- Device access servant
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DevicesI_h
#define _DevicesI_h

#include <device.h>
#include <AstroLoader.h>

namespace snowstar {

class DevicesI : virtual public Devices {
	astro::module::Devices&	_devices;
public:
	// constructors
	DevicesI(astro::module::Devices& devices);
	virtual ~DevicesI();

	// interface methods
	virtual DeviceNameList getDevicelist(devicetype,
			const Ice::Current& current);

public:
	virtual AdaptiveOpticsPrx	getAdaptiveOptics(const std::string& name,
					const Ice::Current& current);
	virtual CameraPrx	getCamera(const std::string& name,
					const Ice::Current& current);
	virtual CcdPrx		getCcd(const std::string&,
					const Ice::Current& current);
	virtual GuidePortPrx	getGuidePort(const std::string&,
					const Ice::Current& current);
	virtual FilterWheelPrx	getFilterWheel(const std::string&,
					const Ice::Current& current);
	virtual CoolerPrx	getCooler(const std::string&,
					const Ice::Current& current);
	virtual FocuserPrx	getFocuser(const std::string&,
					const Ice::Current& current);
	virtual MountPrx	getMount(const std::string&,
					const Ice::Current& current);
};

} // namespace snowstar

#endif /* _DevicesI_h */

