/*
 * AsiLocator.h -- declarations, 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <AstroLoader.h>
#include <AstroCamera.h>
#include <AstroDevice.h>
#include <includes.h>

using namespace astro::camera;
using namespace astro::device;

namespace astro {
namespace camera {
namespace asi {

/**
 * \brief The SBIG CameraLocator class.
 *
 * The SBIG library provides methods to list cameras, this is just
 * an adapter class to the CameraLocator class.
 */
class AsiCameraLocator : public DeviceLocator {
	static int	driveropen;
public:
	AsiCameraLocator();
	virtual ~AsiCameraLocator();
	virtual std::string	getName() const;
	virtual std::string	getVersion() const;
	virtual	std::vector<std::string>	getDevicelist(DeviceName::device_type device = DeviceName::Camera);
protected:
	virtual CameraPtr	getCamera0(const DeviceName& name);
};

} // namespace asi
} // namespace camera
} // namespace astro

