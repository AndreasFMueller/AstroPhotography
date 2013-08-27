/*
 * NetCamera.h -- CORBA based driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NetCamera_h
#define _NetCamera_h

#include <NetLocator.h>
#include <device.hh>
#include <Conversions.h>

namespace astro {
namespace camera {
namespace net {

/**
 * \brief Network client for cameras
 */
class NetCamera : public Camera {
	Astro::Camera_var	_camera;
	bool	_hasfilterwheel;
	bool	_hasguiderport;
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
