/*
 * Camera_impl.h -- Corba Camera implementation wrapper class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Camera_impl_h
#define _Camera_impl_h

#include "../idl/device.hh"
#include <AstroCamera.h>

namespace Astro {

class Camera_impl : public POA_Astro::Camera {
	astro::camera::CameraPtr	_camera;
public:
	inline Camera_impl(astro::camera::CameraPtr camera)
		: _camera(camera) { }
	virtual CORBA::Long	nCcds();
	virtual char	*getName();
	virtual bool	hasFilterWheel();
	virtual bool	hasGuiderPort();
};

} // namespace Astro

#endif /* _Camera_impl_h */
