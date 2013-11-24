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
	Astro::Guider::GuiderState	_state;
	astro::guiding::GuiderPtr	_guider;
	astro::Point	_point;
public:
	inline Guider_impl(astro::guiding::GuiderPtr guider);
	virtual ~Guider_impl() { }

	// state and basic component information
	virtual ::Astro::Guider::GuiderState	getState();
	virtual Camera_ptr	getCamera();
	virtual Ccd_ptr	getCcd();
	virtual GuiderPort_ptr	getGuiderPort();
	virtual ::Astro::GuiderDescriptor	*getDescriptor();

	// setting up the guider
	virtual void	setupGuider(const ::Astro::Exposure& exposure,
		const ::Astro::Point& star);
	virtual Astro::Exposure	getExposure();
	virtual ::Astro::Point	selectedPoint();

	// calibration related methods
	virtual ::Astro::Guider::Calibration	*getCalibration();
	virtual void	useCalibration(const Astro::Guider::Calibration& cal);
	virtual void	startCalibration(::CORBA::Float sensitivity);

	// guding relatedm ethods
	virtual void	startGuiding(::CORBA::Float guidinginterval);
	virtual ::CORBA::Float	getGuidingInterval();
	virtual void	stopGuiding();

	// monitoring
	virtual ShortImage_ptr	mostRecentImage();
	virtual ::Astro::Point	mostRecentOffset();
	virtual ::CORBA::Float	mostRecentDelay();
	virtual ::Astro::Guider::GuiderAction	mostRecentAction();
};

} // namespace Astro

#endif /* _Guider_impl_h */
