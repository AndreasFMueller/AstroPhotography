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

/**
 * \brief The Locator class for Simulator devices
 *
 * All Starlight Express devices are USB devices, so this locator is 
 * essentially a wrapper around a USB context which serves as a factory
 * for the Starlight Express USB devices.
 */
class SimLocator : public astro::device::DeviceLocator {
	CameraPtr	_camera;
	GuiderPortPtr	_guiderport;
	FilterWheelPtr	_filterwheel;
	CoolerPtr	_cooler;
public:
	SimLocator();
	virtual ~SimLocator();

	CameraPtr	camera() { return _camera; }
	GuiderPortPtr	guiderport() { return _guiderport; }
	FilterWheelPtr	filterwheel() { return _filterwheel; }
	CoolerPtr	cooler() { return _cooler; }

	virtual std::string	getName() const;
	virtual std::string	getVersion() const;
	virtual std::vector<std::string>	getDevicelist(
		DeviceLocator::device_type device = DeviceLocator::CAMERA);
protected:
	virtual CameraPtr	getCamera0(const std::string& name);
	virtual FilterWheelPtr	getFilterWheel0(const std::string& name);
	virtual GuiderPortPtr	getGuiderPort0(const std::string& name);
};

} // namespace simulator
} // namespace camera
} // namespace astro

#endif /* _SimLocator_h */
