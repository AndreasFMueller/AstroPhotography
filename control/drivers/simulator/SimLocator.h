/*
 * SimLocator.h -- class to search for simulator cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SimLocator_h
#define _SimLocator_h

#include <AstroLocator.h>
#include <AstroCamera.h>

namespace astro {
namespace camera {
namespace simulator {

class SimCamera;
class SimCcd;
class SimFilterWheel;
class SimGuiderPort;
class SimCooler;
class SimFocuser;

/**
 * \brief The Locator class for Simulator devices
 *
 * The simulator devices all are singletons. The locator keeps a pointer
 * to these devices.
 */
class SimLocator : public astro::device::DeviceLocator {
	CameraPtr	_camera;
	CcdPtr		_ccd;
	GuiderPortPtr	_guiderport;
	FilterWheelPtr	_filterwheel;
	CoolerPtr	_cooler;
	FocuserPtr	_focuser;

	SimLocator(const SimLocator& other);
	SimLocator&	operator=(const SimLocator& other);
public:
	SimLocator();
	virtual ~SimLocator();

	CameraPtr	camera() { return _camera; }
	CcdPtr	ccd() { return _ccd; }
	GuiderPortPtr	guiderport() { return _guiderport; }
	FilterWheelPtr	filterwheel() { return _filterwheel; }
	CoolerPtr	cooler() { return _cooler; }
	FocuserPtr	focuser() { return _focuser; }

	SimCamera	*simcamera();
	SimCcd		*simccd();
	SimGuiderPort	*simguiderport();
	SimFilterWheel	*simfilterwheel();
	SimCooler	*simcooler();
	SimFocuser	*simfocuser();

	virtual std::string	getName() const;
	virtual std::string	getVersion() const;
	virtual std::vector<std::string>	getDevicelist(
		DeviceName::device_type device = DeviceName::Camera);
protected:
	virtual CameraPtr	getCamera0(const DeviceName& name);
	virtual CcdPtr		getCcd0(const DeviceName& name);
	virtual FilterWheelPtr	getFilterWheel0(const DeviceName& name);
	virtual GuiderPortPtr	getGuiderPort0(const DeviceName& name);
	virtual CoolerPtr	getCooler0(const DeviceName& name);
	virtual FocuserPtr	getFocuser0(const DeviceName& name);
};

} // namespace simulator
} // namespace camera
} // namespace astro

#endif /* _SimLocator_h */
