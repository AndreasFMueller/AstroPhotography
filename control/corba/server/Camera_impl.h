/*
 * Camera_impl.h -- Corba Camera implementation wrapper class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Camera_impl_h
#define _Camera_impl_h

#include <camera.hh>
#include <AstroCamera.h>

namespace Astro {

/**
 * \brief Camera servant definition
 */
class Camera_impl : public POA_Astro::Camera {
	astro::camera::CameraPtr	_camera;
	std::vector<astro::camera::CcdPtr>	ccds;
	astro::camera::FilterWheelPtr	filterwheel;
	astro::camera::GuiderPortPtr	guiderport;
public:
	typedef astro::camera::Camera	device_type;
	Camera_impl(astro::camera::CameraPtr camera);
	virtual char	*getName();

	// access to CCDs of a camera (there may be more than one)
	virtual CORBA::Long	nCcds();
	virtual CcdInfo	*getCcdinfo(::CORBA::Long ccdid);
	virtual Ccd_ptr	getCcd(::CORBA::Long ccdid);

	// access to the filter wheel
	virtual bool	hasFilterWheel();
	virtual FilterWheel_ptr	getFilterWheel();

	// access to the guider port
	virtual bool	hasGuiderPort();
	virtual GuiderPort_ptr	getGuiderPort();
};

} // namespace Astro

#endif /* _Camera_impl_h */
