/*
 * SxCamera.h -- starlight express camera declarations
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxCamera_h
#define _SxCamera_h

#include <AstroCamera.h>
#include <AstroUSB.h>

using namespace astro::camera;
using namespace astro::usb;

namespace astro {
namespace camera {
namespace sx {

class SxCamera : public Camera {
	DevicePtr	deviceptr;
	uint16_t	model;
	uint16_t	product;
public:
	DevicePtr	getDevicePtr();
	SxCamera(DevicePtr& devptr);
	virtual ~SxCamera();
	virtual CcdPtr	getCcd(int id);
};


} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxCamera_h */
