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

class SimAdaptiveOptics;
class SimCamera;
class SimCcd;
class SimFilterWheel;
class SimGuidePort;
class SimCooler;
class SimFocuser;
class SimMount;

/**
 * \brief The Locator class for Simulator devices
 *
 * The simulator devices all are singletons. The locator keeps a pointer
 * to these devices.
 */
class SimLocator : public astro::device::DeviceLocator {
	AdaptiveOpticsPtr	_adaptiveoptics;
	CameraPtr	_camera;
	CcdPtr		_ccd;
	GuidePortPtr	_guideport;
	FilterWheelPtr	_filterwheel;
	CoolerPtr	_cooler;
	FocuserPtr	_focuser;
	astro::device::MountPtr	_mount;

	SimLocator(const SimLocator& other);
	SimLocator&	operator=(const SimLocator& other);
public:
	SimLocator();
	virtual ~SimLocator();

	AdaptiveOpticsPtr	adaptiveoptics() { return _adaptiveoptics; }
	CameraPtr	camera() { return _camera; }
	CcdPtr	ccd() { return _ccd; }
	GuidePortPtr	guideport() { return _guideport; }
	FilterWheelPtr	filterwheel() { return _filterwheel; }
	CoolerPtr	cooler() { return _cooler; }
	FocuserPtr	focuser() { return _focuser; }
	astro::device::MountPtr	mount() { return _mount; }

	SimAdaptiveOptics	*simadaptiveoptics();
	SimCamera	*simcamera();
	SimCcd		*simccd();
	SimGuidePort	*simguideport();
	SimFilterWheel	*simfilterwheel();
	SimCooler	*simcooler();
	SimFocuser	*simfocuser();
	SimMount	*simmount();

	virtual std::string	getName() const;
	virtual std::string	getVersion() const;
	virtual std::vector<std::string>	getDevicelist(
		DeviceName::device_type device = DeviceName::Camera);
protected:
	virtual AdaptiveOpticsPtr	getAdaptiveOptics0(const DeviceName& name);
	virtual CameraPtr	getCamera0(const DeviceName& name);
	virtual CcdPtr		getCcd0(const DeviceName& name);
	virtual FilterWheelPtr	getFilterWheel0(const DeviceName& name);
	virtual GuidePortPtr	getGuidePort0(const DeviceName& name);
	virtual CoolerPtr	getCooler0(const DeviceName& name);
	virtual FocuserPtr	getFocuser0(const DeviceName& name);
	virtual astro::device::MountPtr	getMount0(const DeviceName& name);
};

} // namespace simulator
} // namespace camera
} // namespace astro

#endif /* _SimLocator_h */
