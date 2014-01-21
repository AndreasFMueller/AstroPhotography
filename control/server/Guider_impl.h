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
#include <MonitorChannel.h>

namespace Astro {

/**
 * \brief Implementation class for the Guider class
 *
 * The Guider interface is rather complex.
 */
class Guider_impl : public POA_Astro::Guider {
	astro::guiding::GuiderPtr	_guider;
	astro::Point	_point;
	astro::guiding::TrackerPtr	getTracker();

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
private:
	int	calibrationid;
public:
	virtual ::Astro::Calibration	*getCalibration();
	virtual void	useCalibration(CORBA::Long calid);
	virtual void	startCalibration(::CORBA::Float sensitivity);
	virtual void	cancelCalibration();
	virtual CORBA::Double	calibrationProgress();
	virtual bool	waitCalibration(CORBA::Double timeout);

private:
	int	guidingrunid;
public:
	// guiding related methods
	virtual void	startGuiding(::CORBA::Float guidinginterval);
	virtual ::CORBA::Float	getGuidingInterval();
	virtual void	stopGuiding();

	// monitoring
	virtual Image_ptr	mostRecentImage();
	virtual Astro::TrackingPoint	mostRecentTrackingPoint();
	virtual Astro::TrackingHistory	*getTrackingHistory(::CORBA::Long guiderunid);

	//////////////////////////////////////////////////////////////////////
	// callback interface for monitoring
private:
	MonitorChannel< ::Astro::TrackingMonitor, ::Astro::TrackingPoint>
		trackinginfochannel;
public:
	virtual ::CORBA::Long	registerMonitor(TrackingMonitor_ptr monitor);
	virtual void	unregisterMonitor(::CORBA::Long monitorid);
	void	update(const Astro::TrackingPoint& trackinginfo);
	void	tracking_stop();


	//////////////////////////////////////////////////////////////////////
	// callback interface for tracking images
private:
	MonitorChannel< ::Astro::TrackingImageMonitor, ::Astro::TrackingImage>
		trackingimagechannel;
public:
	void	update(const ::Astro::TrackingImage& trackingimage);
	void	trackingimage_stop();
	virtual ::CORBA::Long	registerImageMonitor(
			TrackingImageMonitor_ptr imagemonitor);
	virtual void	unregisterImageMonitor(::CORBA::Long imagemonitorid);

	//////////////////////////////////////////////////////////////////////
	// callback interface for calibration points
private:
	MonitorChannel<::Astro::CalibrationMonitor, ::Astro::CalibrationPoint>
		calibrationchannel;
public:
	void	update(const ::Astro::CalibrationPoint& calibrationpoint);
	void	calibration_stop();

	// callback for calibration
	virtual ::CORBA::Long	registerCalibrationMonitor(
			CalibrationMonitor_ptr monitor);
	virtual void	unregisterCalibrationMonitor(::CORBA::Long monitorid);
};

} // namespace Astro

#endif /* _Guider_impl_h */
