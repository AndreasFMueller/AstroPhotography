/*
 * SxLocator.h -- class to search for starlight express cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxLocator_h
#define _SxLocator_h

#include <AstroLocator.h>
#include <AstroCamera.h>
#include <AstroUSB.h>

using namespace astro::usb;
using namespace astro::device;

namespace astro {
namespace camera {
namespace sx {

/**
 * \brief The Locator class for Starlight Express devices
 *
 * All Starlight Express devices are USB devices, so this locator is 
 * essentially a wrapper around a USB context which serves as a factory
 * for the Starlight Express USB devices.
 *
 * Since it is not a good idea for a camera to be poked by multiple
 * threads, this class makes the assumption that it has only a single
 * instance. This is what the module repository logic ensures, so it
 * would be overkill to add singleton semantics to this class. However,
 * the class has to protect against multiple threads working on the
 * same devices.
 *
 * To properly identify an SX device, it has to be opened as a USB
 * device. This leads to the problem in a multithreaded situation that
 * some thread might want to open a device that is already open.
 * We solve this problem by keeping a map of usb devices in the locator.
 * So normally the device does not need to be constructed more than
 * once. But it is kept in the map so it can easily be retrieved and
 * opened if needed.
 */
class SxCameraLocator : public DeviceLocator {
	Context	context;
	// mutext to pretect the device map
	std::recursive_mutex	_mutex;
	// currently unused
	typedef std::map<std::string, DevicePtr>	device_map_t;
	device_map_t	_sxdevices;
	// add a device to the device
	void	addname(std::vector<std::string> *names,
			DeviceName::device_type device,
			usb::DevicePtr devptr);
public:
	static std::mutex	_hid_mutex;
	SxCameraLocator();
	virtual ~SxCameraLocator();
	virtual std::string	getName() const;
	virtual std::string	getVersion() const;
	virtual std::vector<std::string>	getDevicelist(DeviceName::device_type device = DeviceName::Camera);
protected:
	virtual CameraPtr	getCamera0(const DeviceName& name);
	virtual CcdPtr		getCcd0(const DeviceName& name);
	virtual GuidePortPtr	getGuidePort0(const DeviceName& name);
	virtual CoolerPtr	getCooler0(const DeviceName& name);
	virtual AdaptiveOpticsPtr	getAdaptiveOptics0(const DeviceName& name);
	virtual FilterWheelPtr	getFilterWheel0(const DeviceName& name);
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxLocator_h */
