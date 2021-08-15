/*
 * GuiderI.h -- guider servant definition
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuiderI_h
#define _GuiderI_h

#include <guider.h>
#include <AstroGuiding.h>
#include <AstroPersistence.h>
#include <CallbackHandler.h>
#include <AstroDebug.h>
#include <AstroProject.h>
#include "StatisticsI.h"
#include "RepositoryUser.h"

namespace snowstar {

template<>
void	callback_adapter<TrackingMonitorPrx>(TrackingMonitorPrx p,
		const astro::callback::CallbackDataPtr d);

template<>
void	callback_adapter<ImageMonitorPrx>(ImageMonitorPrx p,
		const astro::callback::CallbackDataPtr d);

template<>
void	callback_adapter<CalibrationMonitorPrx>(CalibrationMonitorPrx p,
		const astro::callback::CallbackDataPtr d);

template<>
void	callback_adapter<CalibrationImageMonitorPrx>(
		CalibrationImageMonitorPrx p,
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
class GuiderI : virtual public Guider, virtual public RepositoryUser, public StatisticsI {
	astro::guiding::GuiderPtr	guider;
	// some infrastructure members we need 
	Point	_point;
	TrackerMethod	_tracker_method;
	astro::guiding::FilterMethod	_filter_method;
	astro::guiding::TrackerPtr	getTracker();

	// callbacks that we need to remove when this object is destroyed
	astro::callback::CallbackPtr	_imagecallback;
	astro::callback::CallbackPtr	_calibrationcallback;
	astro::callback::CallbackPtr	_trackingcallback;
	astro::callback::CallbackPtr	_calibrationimagecallback;
	astro::callback::CallbackPtr	_backlashcallback;

	// public interface starts here
public:
	// construction and destruction
	GuiderI(astro::guiding::GuiderPtr _guider);
	virtual ~GuiderI();

	// get current state of the guider
	virtual GuiderState getState(const Ice::Current& current);

	// devices making up the guider
	virtual CcdPrx getCcd(const Ice::Current& current);
	virtual GuidePortPrx getGuidePort(const Ice::Current& current);
	virtual std::string getInstrumentName(const Ice::Current& current);

	// get parameters
	virtual Ice::Float	getFocallength(const Ice::Current& current);
	virtual	Ice::Float	getGuiderate(const Ice::Current& current);

	// parametrization of the guiding process
	virtual void setExposure(const Exposure&, const Ice::Current& current);
	virtual Exposure getExposure(const Ice::Current& current);
	virtual void setStar(const Point&, const Ice::Current& current);
	virtual Point getStar(const Ice::Current& current);
	virtual void setTrackerMethod(const TrackerMethod,
			const Ice::Current& current);
	virtual TrackerMethod	getTrackerMethod(const Ice::Current& current);

	// choose calibration
	virtual void useCalibration(Ice::Int, bool, const Ice::Current& current);
	virtual Calibration getCalibration(ControlType,
				const Ice::Current& current);
	virtual bool calibrationFlipped(ControlType,
				const Ice::Current& current);
	virtual void flipCalibration(ControlType, const Ice::Current& current);
	virtual bool calibrationMeridianFlipped(ControlType,
				const Ice::Current& current);
	virtual void meridianFlipCalibration(ControlType,
				const Ice::Current& current);
	virtual void unCalibrate(ControlType, const Ice::Current& current);

	// control calibration process
	virtual Ice::Int startCalibration(ControlType caltype,
				Ice::Float gridpixels, bool east,
				const Ice::Current& current);
	virtual Ice::Double calibrationProgress(const Ice::Current& current);
	virtual void cancelCalibration(const Ice::Current& current);
	virtual bool waitCalibration(Ice::Double, const Ice::Current& current);

	// control the filtering method
	virtual void setFilterMethod(const FilterMethod,
			const Ice::Current& current);
	virtual FilterMethod	getFilterMethod(const Ice::Current& current);

	// control guiding process
	virtual void startGuiding(Ice::Float, Ice::Float, bool stepping,
			const Ice::Current& current);
	virtual Ice::Float getGuidingInterval(const Ice::Current& current);
	virtual Ice::Float getFilterParameter(int dir,
			const Ice::Current& current);
	virtual void setFilterParameter(int dir, Ice::Float value,
			const Ice::Current& current);
	virtual void stopGuiding(const Ice::Current& current);

	// methods related to dithering
	virtual void	setDither(const Point& point,
				const Ice::Current& current);
	virtual Point	getDither(const Ice::Current& current);
	virtual void	setDitherArcsec(double arcsec,
				const Ice::Current& current);

	// recent tracking data
	virtual ImagePrx mostRecentImage(const Ice::Current& current);
	virtual TrackingPoint mostRecentTrackingPoint(
			const Ice::Current& current);
	virtual TrackingHistory getTrackingHistory(Ice::Int,
			const Ice::Current& current);
	virtual TrackingHistory getTrackingHistoryType(Ice::Int,
			ControlType type, const Ice::Current& current);
	virtual TrackingSummary	getTrackingSummary(const Ice::Current& current);

	// repository
	virtual void	setRepositoryName(const std::string& reponame,
				const Ice::Current& current);
	virtual std::string	getRepositoryName(const Ice::Current& current);

	// callback handlers
private:
	SnowCallback<ImageMonitorPrx>	imagecallbacks;
	SnowCallback<TrackingMonitorPrx>	trackingcallbacks;
	SnowCallback<CalibrationMonitorPrx>	calibrationcallbacks;
	SnowCallback<CalibrationImageMonitorPrx>	calibrationimagecallbacks;
	SnowCallback<BacklashMonitorPrx>	backlashmonitorcallbacks;

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

	// methods used for dark image acquisition
	virtual void	startDarkAcquire(double exposuretime, int imagecount,
				double badpixellimit,
				const Ice::Current& current);
	virtual ImagePrx	darkImage(const Ice::Current& current);
	virtual bool	hasDark(const Ice::Current& current);
	virtual bool	useDark(const Ice::Current& current);
	virtual void	setUseDark(bool usedark, const Ice::Current& current);

	virtual bool	interpolate(const Ice::Current& current);
	virtual void	setInterpolate(bool interpolate,
				const Ice::Current& current);

	virtual void	startFlatAcquire(double exposuretime, int imagecount,
				bool useDark, const Ice::Current& current);
	virtual ImagePrx	flatImage(const Ice::Current& current);
	virtual bool	hasFlat(const Ice::Current& current);
	virtual bool	useFlat(const Ice::Current& current);
	virtual void	setUseFlat(bool useflat, const Ice::Current& current);

	virtual void	registerCalibrationImageMonitor(
				const Ice::Identity& calibrationimagecallback,
				const Ice::Current& current);
	virtual void	unregisterCalibrationImageMonitor(
				const Ice::Identity& calibrationimagecallback,
				const Ice::Current& current);
	void	calibrationImageUpdate(const astro::callback::CallbackDataPtr data);

	// methods use to acquire images
	virtual void	startImaging(const Exposure& exposure,
				const Ice::Current& current);
	virtual ImagePrx getImage(const Ice::Current& current);

	// methods for backlash characterization
	virtual void	startBacklash(double interval,
				BacklashDirection direction,
				const Ice::Current& current);
	virtual void	stopBacklash(const Ice::Current& current);
	virtual BacklashDirection	getBacklashDirection(const Ice::Current& current);
	virtual BacklashData	getBacklashData(const Ice::Current& current);
	virtual void	setLastPoints(int n, const Ice::Current& current);

	virtual void	registerBacklashMonitor(
				const Ice::Identity& backlashcallback,
				const Ice::Current& current);
	virtual void	unregisterBacklashMonitor(
				const Ice::Identity& backlashcallback,
				const Ice::Current& current);
	void	backlashUpdate(const astro::callback::CallbackDataPtr data);
};

/**
 * \brief Adapter class for calibration callback
 */
class GuiderICalibrationCallback : public astro::callback::Callback {
	GuiderI&	_guider;
public:
	GuiderICalibrationCallback(GuiderI& guider) : _guider(guider) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"calibration callback %p created", this);
	}
	virtual astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration callback called");
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
	virtual astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data) {
		_guider.trackingUpdate(data);
		return data;
	}
};

/**
 * \brief Adapter class for image callback
 */
class GuiderIImageCallback : public astro::callback::Callback {
	GuiderI&	_guider;
public:
	GuiderIImageCallback(GuiderI& guider) : _guider(guider) { }
	virtual astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data) {
		_guider.trackingImageUpdate(data);
		return data;
	}
};

/**
 * \brief Adapter class for calibration image callback
 */
class GuiderICalibrationImageCallback : public astro::callback::Callback {
	GuiderI&	_guider;
public:
	GuiderICalibrationImageCallback(GuiderI& guider) : _guider(guider) { }
	virtual astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data) {
		_guider.calibrationImageUpdate(data);
		return data;
	}
};

/**
 * \brief Adapter class for Backlash  callback
 */
class GuiderIBacklashCallback : public astro::callback::Callback {
	GuiderI&	_guider;
public:
	GuiderIBacklashCallback(GuiderI& guider) : _guider(guider) { }
	virtual astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data) {
		_guider.backlashUpdate(data);
		return data;
	}
};

} // namespace snowstar

#endif /* _GuiderI_h */
