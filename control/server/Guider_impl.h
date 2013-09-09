/*
 * Guider_impl.h -- Guider servant implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Guider_impl_h
#define _Guider_impl_h

namespace Astro {

class Guider_impl : public POA_Astro::Guider {
public:
	inline Guider_impl() { }
	virtual ~Guider_impl() { }
	virtual Guider::GuiderState	getState();
	virtual _obj_ref_Camera	*getCamera();
	virtual _obj_ref_Ccd	*getCcd();
	virtual _obj_ref_GuiderPort	*getGuiderPort();
	virtual void	setupGuider(const ::Astro::Guider::ImageRectangle& rectangle, const ::Astro::Guider::Point& star, ::CORBA::Float exposuretime);
	virtual Astro::Guider::ImageRectangle selectedArea();
	virtual Astro::Guider::Point	selectedPoint();
	virtual ::CORBA::Float	exposuretime();
	virtual Astro::Guider::Calibration	*getCalibration();
	virtual void	useCalibration(Astro::Guider::Calibration& cal);
	virtual void	startCalibration(::CORBA::Float sensitivity);
	virtual void	startGuding(::CORBA::Float guidinginterval);
	virtual void	stopGuiding();
	virtual _obj_ref_ShortImage	mostRecentImage();
	virtual Astro::Guider::Point	mostRecentOffset();
	virtual ::CORBA::Float	mostRecentDelay();
	virtual Astro::Guider::GuiderAction	mostRecentAction();
};

} // namespace Astro

#endif /* _Guider_impl_h */
