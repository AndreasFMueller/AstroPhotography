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

class DevicesI : public Devices {
	astro::module::Devices&	_devices;
public:
	// constructors
	DevicesI(astro::module::Devices& devices);
	virtual ~DevicesI();

	// conversion methods
static std::vector<std::string>	convert(
		const astro::module::Devices::devicelist& list);

static devicetype	convert(const astro::DeviceName::device_type& type);
static astro::DeviceName::device_type	convert(const devicetype& type);

	// interface methods
	virtual DeviceNameList getDevicelist(devicetype,
			const Ice::Current& current);

private:
	Ice::ObjectPrx	getObject(const std::string& name,
				const Ice::Current& current);
public:
	virtual CameraPrx	getCamera(const std::string& name,
					const Ice::Current& current);
	virtual CcdPrx		getCcd(const std::string&,
					const Ice::Current& current);
	virtual GuiderPortPrx	getGuiderPort(const std::string&,
					const Ice::Current& current);
	virtual FilterWheelPrx	getFilterWheel(const std::string&,
					const Ice::Current& current);
	virtual CoolerPrx	getCooler(const std::string&,
					const Ice::Current& current);
	virtual FocuserPrx	getFocuser(const std::string&,
					const Ice::Current& current);
};

} // namespace snowstar

#endif /* _DevicesI_h */

