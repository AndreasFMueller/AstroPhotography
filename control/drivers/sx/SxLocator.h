/*
 * SxLocator.h -- class to search for starlight express cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxLocator_h
#define _SxLocator_h

#include <AstroCamera.h>
#include <AstroUSB.h>

using namespace astro::usb;

namespace astro {
namespace camera {
namespace sx {

class SxCameraLocator : public CameraLocator {
	Context	context;
public:
	SxCameraLocator();
	virtual ~SxCameraLocator();
	virtual std::vector<std::string>	getCameralist();
	virtual CameraPtr	getCamera(const std::string& name);
	virtual CameraPtr	getCamera(size_t index);
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxLocator_h */
