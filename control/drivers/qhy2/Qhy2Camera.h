/*
 * Qhy2Camera.h -- QHY camera interface
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Qhy2Camera_h
#define _Qhy2Camera_h

#include <AstroCamera.h>
#include <AstroUSB.h>
#include <qhyccd.h>

namespace astro {
namespace camera {
namespace qhy2 {

/**
 * \brief QHY Camera class
 *
 * This is mainly a wrapper class that is used to forward commands to
 * the device class from the QHY library
 */
class Qhy2Camera : public astro::camera::Camera {
public:
	Qhy2Camera(usb::DevicePtr& devptr);
	virtual ~Qhy2Camera();
protected:
	virtual CcdPtr	getCcd0(size_t id);
	// we currently don't know how to control the guider port of
	// the camera from linux, so we do not offer a guider port
//	virtual GuidePortPtr	getGuidePort0();
};

} // namespace qhy2
} // namespace camera
} // namespace astro

#endif /* _Qhy2Camera_h */
