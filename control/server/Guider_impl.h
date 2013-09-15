/*
 * Guider_impl.h -- Guider servant implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Guider_impl_h
#define _Guider_impl_h

#include <guider.hh>
#include <AstroGuiding.h>

namespace Astro {

class Guider_impl : public POA_Astro::Guider {
	astro::camera::CameraPtr	_camera;
	astro::camera::GuiderPortPtr	_guiderport;
	astro::guiding::GuiderPtr	_guider;
public:
	inline Guider_impl(astro::guiding::GuiderPtr guider) : _guider(guider) { }
	virtual ~Guider_impl() { }
	virtual ::Astro::Guider::GuiderState	getState();
	virtual Camera_ptr	getCamera();
	virtual Ccd_ptr	getCcd();
	virtual GuiderPort_ptr	getGuiderPort();
	virtual void	setupGuider(const ::Astro::ImageRectangle& rectangle, const ::Astro::Guider::Point& star, ::CORBA::Float exposuretime);
	virtual ImageRectangle selectedArea();
	virtual ::Astro::Guider::Point	selectedPoint();
	virtual ::CORBA::Float	exposureTime();
	virtual ::Astro::Guider::Calibration	*getCalibration();
	virtual void	useCalibration(const Astro::Guider::Calibration& cal);
	virtual void	startCalibration(::CORBA::Float sensitivity);
	virtual void	startGuiding(::CORBA::Float guidinginterval);
	virtual ::CORBA::Float	getGuidingInterval();
	virtual void	stopGuiding();
	virtual ShortImage_ptr	mostRecentImage();
	virtual ::Astro::Guider::Point	mostRecentOffset();
	virtual ::CORBA::Float	mostRecentDelay();
	virtual ::Astro::Guider::GuiderAction	mostRecentAction();
};

} // namespace Astro

#endif /* _Guider_impl_h */
