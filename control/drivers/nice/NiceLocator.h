/*
 * NiceLocator.h -- Device locator class for ZeroC ICE
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NiceLocator_h
#define _NiceLocator_h

#include <includes.h>
#include <AstroLocator.h>
#include <AstroCamera.h>
#include <ServiceDiscovery.h>
#include <device.h>

namespace astro {
namespace camera {
namespace nice {

/**
 * \brief ICE network client for locators
 */
class NiceLocator : public astro::device::DeviceLocator {
	astro::discover::ServiceDiscoveryPtr	discovery;
public:
	NiceLocator();
	virtual ~NiceLocator();

	virtual std::string	getName() const;
	virtual std::string	getVersion() const;
private:
	std::vector<std::string>	getDevicelist(
		DeviceName::device_type device,
		snowstar::DriverModulePrx module);
	std::vector<std::string>	getDevicelist(
		DeviceName::device_type device,
		const astro::discover::ServiceKey& key);
public:
	virtual std::vector<std::string>	getDevicelist(DeviceName::device_type device = DeviceName::Camera);
protected:
	virtual CameraPtr	getCamera0(const DeviceName& name);
	virtual CcdPtr		getCcd0(const DeviceName& name);
	virtual FilterWheelPtr	getFilterWheel0(const DeviceName& name);
	virtual GuiderPortPtr	getGuiderPort0(const DeviceName& name);
	virtual CoolerPtr	getCooler0(const DeviceName& name);
	virtual FocuserPtr	getFocuser0(const DeviceName& name);
	virtual AdaptiveOpticsPtr	getAdaptiveOptics0(const DeviceName& name);
};

} // namespace nice
} // namespace camera
} // namespace astro

#endif /* _NiceLocator_h */
