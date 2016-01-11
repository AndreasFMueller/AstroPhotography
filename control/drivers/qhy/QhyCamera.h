/*
 * QhyCamera.h -- QHY camera interface
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _QhyCamera_h
#define _QhyCamera_h

#include <AstroCamera.h>
#include <AstroUSB.h>
#include <qhylib.h>

namespace astro {
namespace camera {
namespace qhy {

/**
 * \brief QHY Camera class
 *
 * This is mainly a wrapper class that is used to forward commands to
 * the device class from the QHY library
 */
class QhyCamera : public astro::camera::Camera {
	usb::DevicePtr	deviceptr;
	usb::InterfacePtr	interface;
	::qhy::DevicePtr	qhydeviceptr;
	unsigned short	idVendor;
	unsigned short	idProduct;
public:
	QhyCamera(usb::DevicePtr& devptr);
	virtual ~QhyCamera();
protected:
	virtual CcdPtr	getCcd0(size_t id);
	// we currently don't know how to control the guider port of
	// the camera from linux, so we do not offer a guider port
//	virtual GuiderPortPtr	getGuiderPort0();
};

} // namespace qhy
} // namespace camera
} // namespace astro

#endif /* _QhyCamera_h */
