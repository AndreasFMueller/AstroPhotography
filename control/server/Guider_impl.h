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
	astro::guiding::GuiderPtr	_guider;
	astro::Point	_point;
	astro::guiding::TrackerPtr	getTracker();
public:
	Guider_impl(astro::guiding::GuiderPtr guider);
	virtual ~Guider_impl() { }

	// state and basic component information
	virtual ::Astro::Guider::GuiderState	getState();
	virtual Camera_ptr	getCamera();
	virtual Ccd_ptr	getCcd();
	virtual GuiderPort_ptr	getGuiderPort();
	virtual ::Astro::GuiderDescriptor	*getDescriptor();

	// setting up the guider
	virtual Astro::Exposure	getExposure();
	virtual void	setExposure(const Astro::Exposure& exposre);

	virtual Astro::Point	getStar();
	virtual	void	setStar(const Astro::Point& star);

	// calibration related methods
	virtual ::Astro::Guider::Calibration	getCalibration();
	virtual void	useCalibration(const Astro::Guider::Calibration& cal);
	virtual void	startCalibration(::CORBA::Float sensitivity);
	virtual void	cancelCalibration();
	virtual CORBA::Double	calibrationProgress();
	virtual bool	waitCalibration(CORBA::Double timeout);

	// guding related methods
	virtual void	startGuiding(::CORBA::Float guidinginterval);
	virtual ::CORBA::Float	getGuidingInterval();
	virtual void	stopGuiding();

	// monitoring
	virtual Image_ptr	mostRecentImage();
	virtual Astro::Guider::TrackingInfo	mostRecentTrackingInfo();
};

} // namespace Astro

#endif /* _Guider_impl_h */
