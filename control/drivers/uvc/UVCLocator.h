/*
 * UVCLocator.h -- declarations of the camera 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _UVCLocator_h
#define _UVCLocator_h

#include <AstroUSB.h>
#include <AstroCamera.h>

using namespace astro::usb;
using namespace astro::camera;

namespace astro {
namespace camera {
namespace uvc {

/**
 * \brief The UVC camera locator
 *
 * Each UVC camera is also a camera from the point of view of this 
 */
class UVCCameraLocator : public CameraLocator {
	Context	context;
public:
	UVCCameraLocator();
	virtual ~UVCCameraLocator();
	virtual std::vector<std::string>	getCameralist();
	virtual CameraPtr	getCamera(const std::string& name);
	virtual CameraPtr	getCamera(size_t index);
};

} // namespace uvc
} // namespace camera
} // namespace uvc

#endif /* _UVCLocator_h */
