/*
 * NetCamera.h -- CORBA based driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NetCamera_h
#define _NetCamera_h

#include <NetLocator.h>
#include <device.hh>

namespace astro {
namespace camera {
namespace net {

class NetCamera : public Camera {
	Astro::Camera_var	_camera;
public:
	NetCamera(Astro::Camera_var camera);
	virtual CcdPtr	getCcd0(size_t ccdid);
	virtual bool	hasFilterWheel() const;
	virtual FilterWheelPtr	getFilterWheel0();
	virtual bool	hasGuiderPort() const;
	virtual GuiderPortPtr	getGuiderPort0();
};

} // namespace net
} // namespace camera
} // namespace astro

#endif /* _NetCamera_h */
