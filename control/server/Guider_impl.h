/*
 * Guider_impl.h -- Guider servant implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Guider_impl_h
#define _Guider_impl_h

#include <guider.hh>
#include <AstroGuiding.h>
#include <map>

namespace Astro {

class Guider_impl : public POA_Astro::Guider {
	astro::guiding::GuiderPtr	_guider;
	astro::Point	_point;
	astro::guiding::TrackerPtr	getTracker();

	pthread_mutex_t	mutex; // mutex to protect the maps
	typedef	std::map< ::CORBA::Long, TrackingMonitor_var>	monitormap_t;
	monitormap_t	monitors;
	typedef std::map< ::CORBA::Long, TrackingImageMonitor_var>	imagemonitormap_t;
	imagemonitormap_t	imagemonitors;

	int	guidingrunid;
public:
	Guider_impl(astro::guiding::GuiderPtr guider);
	virtual ~Guider_impl();

	// state and basic component information
	virtual ::Astro::Guider::GuiderState	getState();
	std::string	getCameraName() const {
		return _guider->camera()->name().toString();
	}
	virtual Camera_ptr	getCamera();
	virtual Ccd_ptr	getCcd();
	int	getCcdid() const {
		return _guider->ccd()->getInfo().getId();
	}
	virtual GuiderPort_ptr	getGuiderPort();
	std::string	getGuiderPortName() const {
		return _guider->guiderport()->name().toString();
	}
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
	virtual Astro::TrackingInfo	mostRecentTrackingInfo();
	virtual Astro::TrackingHistory	*getTrackingHistory(::CORBA::Long guiderunid);

	// callback interface for monitoring
	virtual ::CORBA::Long	registerMonitor(TrackingMonitor_ptr monitor);
	virtual void	unregisterMonitor(::CORBA::Long monitorid);
	void	update(const Astro::TrackingInfo& trackinginfo);
	void	update_stop();

	// callback interface for image monitoring
	virtual ::CORBA::Long	registerImageMonitor(TrackingImageMonitor_ptr imagemonitor);
	virtual void	unregisterImageMonitor(::CORBA::Long imagemonitorid);
	void	update(const ::Astro::ImageSize& size,
			const ::Astro::ShortSequence_var& imagedata);
};

} // namespace Astro

#endif /* _Guider_impl_h */
