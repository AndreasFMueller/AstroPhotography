/*
 * Ccd_impl.h -- Corba Ccd Implementation wrapper
 *
 * (c) 2013 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _Ccd_impl_h
#define _Ccd_impl_h

#include <camera.hh>
#include <AstroCamera.h>

namespace Astro {

/**
 * \brief Ccd servant definition
 */
class Ccd_impl : public POA_Astro::Ccd {
	astro::camera::CcdPtr	_ccd;
	astro::image::ImagePtr	image;
public:
	// constructor
	inline Ccd_impl(astro::camera::CcdPtr ccd) : _ccd(ccd) { }

	virtual char	*getName();

	// exposure related stuff
	virtual void	startExposure(const ::Astro::Exposure& exp);
	virtual ExposureState	exposureStatus();
	virtual Exposure	getExposure();
	virtual void	cancelExposure();

	// get image method
	Image_ptr	getImage();

	// gain stuff
	virtual ::CORBA::Boolean	hasGain();

	// shutter methods
	virtual ::CORBA::Boolean	hasShutter();
	ShutterState	getShutterState();
	void	setShutterState(ShutterState state);

	// cooler methods
	virtual ::CORBA::Boolean	hasCooler();
	virtual Cooler_ptr	getCooler();
};

} // namespace Astro

#endif /* _Ccd_impl_h */
