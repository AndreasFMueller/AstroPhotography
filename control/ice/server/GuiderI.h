/*
 * GuiderI.h -- guider servant definition
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuiderI_h
#define _GuiderI_h

#include <guider.h>
#include <AstroGuiding.h>
#include <ImageDirectory.h>
#include <AstroPersistence.h>
#include <CallbackHandler.h>

namespace snowstar {

template<>
void	callback_adapter<TrackingMonitorPrx>(TrackingMonitorPrx& p,
		const astro::callback::CallbackDataPtr d);

template<>
void	callback_adapter<TrackingImageMonitorPrx>(TrackingImageMonitorPrx& p,
		const astro::callback::CallbackDataPtr d);

template<>
void	callback_adapter<CalibrationMonitorPrx>(CalibrationMonitorPrx& p,
		const astro::callback::CallbackDataPtr d);

/**
 * \brief Guider servant class
 *
 * The servant class care of relaying commands from the client to the
 * implementation classes in the astro namespace. The callbacks called
 * by the implementation classes are caught by the adapter classes like
 * GuiderITrackingCallback etc and used to persist the data and to send
 * it to callbacks via ICE.
 */
class GuiderI : public Guider {
	astro::guiding::GuiderPtr	guider;
	astro::image::ImageDirectory	imagedirectory;
	astro::persistence::Database	database;
	// some infrastructure members we need 
	Point	_point;
	int	calibrationid;
	int	guidingrunid;
	astro::guiding::TrackerPtr	getTracker();

	// public interface starts here
public:
	// construction and destruction
	GuiderI(astro::guiding::GuiderPtr _guider,
		astro::image::ImageDirectory& imagedirectoy,
		astro::persistence::Database database);
	virtual ~GuiderI();

	// get current state of the guider
	virtual GuiderState getState(const Ice::Current& current);

	// devices making up the guider
	virtual CameraPrx getCamera(const Ice::Current& current);
	virtual CcdPrx getCcd(const Ice::Current& current);
	virtual GuiderPortPrx getGuiderPort(const Ice::Current& current);
	virtual GuiderDescriptor getDescriptor(const Ice::Current& current);

	// parametrization of the guiding process
	virtual void setExposure(const Exposure&, const Ice::Current& current);
	virtual Exposure getExposure(const Ice::Current& current);
	virtual void setStar(const Point&, const Ice::Current& current);
	virtual Point getStar(const Ice::Current& current);

	// choose calibration
	virtual void useCalibration(Ice::Int, const Ice::Current& current);
	virtual Calibration getCalibration(const Ice::Current& current);

	// control calibration process
	virtual void startCalibration(Ice::Float, const Ice::Current& current);
	virtual Ice::Double calibrationProgress(const Ice::Current& current);
	virtual void cancelCalibration(const Ice::Current& current);
	virtual bool waitCalibration(Ice::Double, const Ice::Current& current);

	// control guiding process
	virtual void startGuiding(Ice::Float, const Ice::Current& current);
	virtual Ice::Float getGuidingInterval(const Ice::Current& current);
	virtual void stopGuiding(const Ice::Current& current);

	// recent tracking data
	virtual ImagePrx mostRecentImage(const Ice::Current& current);
	virtual TrackingPoint mostRecentTrackingPoint(
			const Ice::Current& current);
	virtual TrackingHistory getTrackingHistory(Ice::Int,
			const Ice::Current& current);

	// callback handlers
private:
	SnowCallback<TrackingImageMonitorPrx>	trackingimagecallbacks;
	SnowCallback<TrackingImageMonitorPrx>	trackingcallbacks;
	SnowCallback<CalibrationMonitorPrx>	calibrationcallbacks;

	// methods for registration and unregistration of callbacks
public:
	virtual void	registerCalibrationMonitor(
				const Ice::Identity& calibrationcallback,
				const Ice::Current& current);
	virtual void	unregisterCalibrationMonitor(
				const Ice::Identity& calibrationcallback,
				const Ice::Current& current);
	virtual void	registerImageMonitor(
				const Ice::Identity& imagecallback,
				const Ice::Current& current);
	virtual void	unregisterImageMonitor(
				const Ice::Identity& imagecallback,
				const Ice::Current& current);
	virtual void	registerTrackingMonitor(
				const Ice::Identity& trackingcallback,
				const Ice::Current& current);
	virtual void	unregisterTrackingMonitor(
				const Ice::Identity& trackingcallback,
				const Ice::Current& current);

	// methods that are used to forward callback data to the callbacks
	void	trackingUpdate(const astro::callback::CallbackDataPtr data);
	void	calibrationUpdate(const astro::callback::CallbackDataPtr data);
	void	trackingImageUpdate(const astro::callback::CallbackDataPtr data);
};

/**
 * \brief Adapter class for calibration callback
 */
class GuiderICalibrationCallback : public astro::callback::Callback {
	GuiderI&	_guider;
public:
	GuiderICalibrationCallback(GuiderI& guider) : _guider(guider) { }
	astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr& data) {
		_guider.calibrationUpdate(data);
		return data;
	}
};

/**
 * \brief Adapter class for tracking callback
 */
class GuiderITrackingCallback : public astro::callback::Callback {
	GuiderI&	_guider;
public:
	GuiderITrackingCallback(GuiderI& guider) : _guider(guider) { }
	astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr& data) {
		_guider.trackingUpdate(data);
		return data;
	}
};

/**
 * \brief Adapter class for image callback
 */
class GuiderITrackingImageCallback : public astro::callback::Callback {
	GuiderI&	_guider;
public:
	GuiderITrackingImageCallback(GuiderI& guider) : _guider(guider) { }
	astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr& data) {
		_guider.trackingImageUpdate(data);
		return data;
	}
};

} // namespace snowstar

#endif /* _GuiderI_h */
