/*
 * GuiderI.cpp -- guider servern implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuiderI.h>
#include <IceConversions.h>
#include <CameraI.h>
#include <CcdI.h>
#include <GuiderPortI.h>
#include <ImagesI.h>
#include <CalibrationStore.h>
#include <AstroGuiding.h>
#include <TrackingPersistence.h>
#include <TrackingStore.h>

namespace snowstar {

/**
 * \brief calback adapter for Tracking monitor
 */
template<>
void	callback_adapter<TrackingMonitorPrx>(TrackingMonitorPrx& p,
		const astro::callback::CallbackDataPtr data) {
	// check whether the info we got is 
	astro::guiding::TrackingPoint   *trackinginfo
		= dynamic_cast<astro::guiding::TrackingPoint *>(&*data);

	// leave immediately, if there is not Tracking info
	if (NULL == trackinginfo) {
		return;
	}

	// tracking 
	TrackingPoint	trackingpoint = convert(*trackinginfo);
	p->update(trackingpoint);
}

/**
 * \brief calback adapter for Tracking monitor
 */
template<>
void	callback_adapter<ImageMonitorPrx>(ImageMonitorPrx& p,
		const astro::callback::CallbackDataPtr data) {
	// first check whether we really got an image
	astro::callback::ImageCallbackData	*imageptr
		= dynamic_cast<astro::callback::ImageCallbackData *>(&*data);
	if (NULL == imageptr) {
		return;
	}

	// convert the image so that it is understood by the
	// ImageMonitor proxy
	SimpleImage	image;
	image.size = convert(imageptr->image()->size());

	// create an adapter to a shor timage
	astro::image::Image<unsigned short>	*im
		= dynamic_cast<astro::image::Image<unsigned short> *>(
			&*imageptr->image());
	if (NULL == im) {
		debug(LOG_ERR, DEBUG_LOG, 0, "don't know how to handle non short images");
		return;
	}

	// copy all the bytes
	for (int x = 0; x < image.size.width; x++) {
		for (int y = 0; y < image.size.height; y++) {
			unsigned short	value = im->pixel(x, y);
			image.imagedata.push_back(value);
		}
	}

	// now that the image has been created, send it to the callback
	p->update(image);
}

/**
 * \brief calback adapter for Tracking monitor
 */
template<>
void	callback_adapter<CalibrationMonitorPrx>(CalibrationMonitorPrx& p,
		const astro::callback::CallbackDataPtr data) {
	// handle a calibration point callback call
	astro::guiding::CalibrationPointCallbackData	*calibrationpoint
		= dynamic_cast<astro::guiding::CalibrationPointCallbackData *>(&*data);
	if (NULL != calibrationpoint) {
		// convert the calibration point into
		CalibrationPoint	point
			= convert(calibrationpoint->calibrationpoint());
		p->update(point);
		return;
	}

	// handle a completed calibration callback call, by sending the stop
	// signal
	astro::guiding::GuiderCalibrationCallbackData	*calibration
		= dynamic_cast<astro::guiding::GuiderCalibrationCallbackData *>(&*data);
	if (NULL != calibration) {
		p->stop();
		return;
	}
}

/**
 * \brief Constructor for the Guider servant
 */
GuiderI::GuiderI(astro::guiding::GuiderPtr _guider,
	astro::image::ImageDirectory& _imagedirectory,
	astro::persistence::Database _database)
	: guider(_guider), imagedirectory(_imagedirectory),
	  database(_database) {
}

GuiderI::~GuiderI() {
}

GuiderState GuiderI::getState(const Ice::Current& /* current */) {
	return convert(guider->state());
}

CameraPrx GuiderI::getCamera(const Ice::Current& current) {
	std::string	name = guider->getDescriptor().cameraname();
	return CameraI::createProxy(name, current);
}

CcdPrx GuiderI::getCcd(const Ice::Current& current) {
	std::string	ccdname = guider->ccd()->name().toString();
	return CcdI::createProxy(ccdname, current);
}

GuiderPortPrx GuiderI::getGuiderPort(const Ice::Current& current) {
	std::string	name = guider->getDescriptor().guiderportname();
	return GuiderPortI::createProxy(name, current);
}

GuiderDescriptor GuiderI::getDescriptor(const Ice::Current& /* current */) {
	return convert(guider->getDescriptor());
}

void GuiderI::setExposure(const Exposure& exposure,
	const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set exposure: time=%f",
		exposure.exposuretime);
	guider->exposure(convert(exposure));
}

Exposure GuiderI::getExposure(const Ice::Current& /* current */) {
	return convert(guider->exposure());
}

void GuiderI::setStar(const Point& point, const Ice::Current& /* current */) {
	_point = point;
}

Point GuiderI::getStar(const Ice::Current& /* current */) {
	return _point;
}

void GuiderI::useCalibration(Ice::Int calibrationid,
	const Ice::Current& /* current */) {
	// retrieve guider data from the database
	astro::guiding::CalibrationStore	store(database);
	astro::guiding::GuiderCalibration	calibration
		= store.getCalibration(calibrationid);

	// install calibration data in the guider
	guider->calibration(calibration);
}

Calibration GuiderI::getCalibration(const Ice::Current& /* current */) {
	Calibration	result;
	result.id = calibrationid;

	// get the calibration record from the database
	astro::guiding::CalibrationTable	t(database);
	astro::guiding::CalibrationRecord	cal = t.byid(calibrationid);
	result.timeago = converttime(cal.when);
	result.guider.cameraname = cal.camera;
	result.guider.ccdid = cal.ccdid;
	result.guider.guiderportname = cal.guiderport;
	for (int i = 0; i < 6; i++) {
		result.coefficients[i] = cal.a[i];
	}

	// use database to retrieve calibration, containing coefficients
	// and points
	astro::guiding::CalibrationStore	store(database);
	astro::guiding::GuiderCalibration	calibration
		= store.getCalibration(calibrationid);
	for (auto ptr = calibration.begin(); ptr != calibration.end(); ptr++) {
		result.points.push_back(convert(*ptr));
	}

	// everything copied, return it
	return result;
}

/**
 * \brief Start a calibration for a given focal length
 *
 * The focal length is the only piece of information that we can not
 * get from anywhere else, so it has to be specified
 */
void GuiderI::startCalibration(Ice::Float focallength,
	const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start calibration");

	// prepare a calibration record
	astro::guiding::Calibration	calibration;
	calibration.camera = guider->cameraname();
	calibration.ccdid = guider->ccdid();
	calibration.guiderport = guider->guiderportname();

	// create an entry in the calibration table
	astro::guiding::CalibrationStore	store(database);
	calibrationid = store.addCalibration(calibration);
	
	// callback stuff
	GuiderICalibrationCallback	*callback
		= new GuiderICalibrationCallback(*this);
	guider->calibrationcallback = astro::callback::CallbackPtr(callback);

	// get the pixel size from the 
	double	pixelsize = guider->getPixelsize();

	// construct a tracker
	astro::guiding::TrackerPtr	tracker
		= guider->getTracker(convert(_point));

	// start the calibration
	guider->startCalibration(tracker, focallength, pixelsize);
}

/**
 * \brief Retrieve the current progress figure of the calibration
 */
Ice::Double GuiderI::calibrationProgress(const Ice::Current& /* current */) {
	return guider->calibrationProgress();
}

/**
 * \brief cancel the current calibration process
 */
void GuiderI::cancelCalibration(const Ice::Current& /* current */) {
	guider->cancelCalibration();
}

/**
 * \brief Wait for the calibration to complete
 */
bool GuiderI::waitCalibration(Ice::Double timeout,
	const Ice::Current& /* current */) {
	return guider->waitCalibration(timeout);
}

/**
 * \brief build a tracker
 */
astro::guiding::TrackerPtr	 GuiderI::getTracker() {
	astro::camera::Exposure	exposure = guider->exposure();
	astro::Point	d = convert(_point) - exposure.frame.origin();
	astro::image::ImagePoint	trackerstar(d.x(), d.y());
	astro::image::ImageRectangle	trackerrectangle(exposure.frame.size());
	astro::guiding::TrackerPtr	tracker(
		new astro::guiding::StarTracker(trackerstar,
			trackerrectangle, 10));
	return tracker;
}

/**
 * \brief Start guiding
 */
void GuiderI::startGuiding(Ice::Float guidinginterval,
		const Ice::Current& /* current */) {
	// build a tracker
	astro::guiding::TrackerPtr	tracker = getTracker();

	// start guiding
	guider->startGuiding(tracker, guidinginterval);

	// install a callback in the guider
	GuiderITrackingCallback	*callback = new GuiderITrackingCallback(*this);
	guider->trackingcallback = astro::callback::CallbackPtr(callback);
}

Ice::Float GuiderI::getGuidingInterval(const Ice::Current& /* current */) {
	return guider->getInterval();
}

void GuiderI::stopGuiding(const Ice::Current& /* current */) {
	guider->stopGuiding();

	// send the clients that guiding was stopped
	trackingcallbacks.stop();
	imagecallbacks.stop();

	// remove the callback
	guider->trackingcallback.reset();
}

ImagePrx GuiderI::mostRecentImage(const Ice::Current& current) {
	// retrieve image
	astro::image::ImagePtr	image = guider->mostRecentImage;
	if (!image) {
		throw NotFound("no image available");
	}

	// store image in image directory
	std::string	filename = imagedirectory.save(image);

	// return a proxy for the image
	return getImage(filename, image->bytesPerPixel(), current);
}

TrackingPoint GuiderI::mostRecentTrackingPoint(const Ice::Current& /* current */) {
	if (astro::guiding::guiding != guider->state()) {
		throw BadState("not currently guiding");
	}

	// get info from the guider
	double	lastaction;
	astro::Point	offset;
	astro::Point	activation;
	guider->lastAction(lastaction, offset, activation);
	
	// construct a tracking point
	TrackingPoint	result;
	result.timeago = converttime(lastaction);
	result.trackingoffset = convert(offset);
	result.activation = convert(activation);
	return result;
}

TrackingHistory GuiderI::getTrackingHistory(Ice::Int id,
	const Ice::Current& /* current */) {
	astro::guiding::TrackingStore	store(database);
	return convert(store.get(id));
}

/**
 * \brief Register a callback for the calibration process
 */
void	GuiderI::registerCalibrationMonitor(const Ice::Identity& calibrationcallback, const Ice::Current& current) {
	calibrationcallbacks.registerCallback(calibrationcallback, current);
}

/**
 * \brief Unregister a callback for the calibration process
 */
void	GuiderI::unregisterCalibrationMonitor(const Ice::Identity& calibrationcallback, const Ice::Current& current) {
	calibrationcallbacks.unregisterCallback(calibrationcallback, current);
}

/**
 * \brief Register a callback for images taken during the process
 */
void    GuiderI::registerImageMonitor(const Ice::Identity& imagecallback,
		const Ice::Current& current) {
	imagecallbacks.registerCallback(imagecallback, current);
}

/**
 * \brief Unregister a callback for images
 */
void    GuiderI::unregisterImageMonitor(const Ice::Identity& imagecallback,
		const Ice::Current& current) {
	imagecallbacks.unregisterCallback(imagecallback, current);
}

/**
 * \brief Register a callback for monitoring the tracking
 */
void    GuiderI::registerTrackingMonitor(const Ice::Identity& trackingcallback,
		const Ice::Current& current) {
	trackingcallbacks.registerCallback(trackingcallback, current);
}

/**
 * \brief Unregister a callback for monitoring the tracking
 */
void    GuiderI::unregisterTrackingMonitor(const Ice::Identity& trackingcallback,
		const Ice::Current& current) {
	trackingcallbacks.unregisterCallback(trackingcallback, current);
}

/**
 * \brief Handle a tracking update
 *
 * This method needs to be called by the callback installed in the guider
 */
void	GuiderI::trackingUpdate(const astro::callback::CallbackDataPtr data) {
	astro::guiding::TrackingPoint	*trackinginfo
		= dynamic_cast<astro::guiding::TrackingPoint *>(&*data);
	if (NULL != trackinginfo) {
		// write tracking update to the database
		astro::guiding::TrackingTable	tracking(database);
		astro::guiding::TrackingPointRecord	tp(-1, guidingrunid,
			*trackinginfo);
		tracking.add(tp);
	}

	// tell the clients that data has changed
	trackingcallbacks(data);
}

/**
 * \brief Handle a new image from the tracking process
 */
void	GuiderI::trackingImageUpdate(const astro::callback::CallbackDataPtr data) {
	imagecallbacks(data);
}

/**
 * \brief Handle an update from the calibration process
 */
void	GuiderI::calibrationUpdate(const astro::callback::CallbackDataPtr data) {
	// handle various information from the calibration process
	astro::guiding::CalibrationPointCallbackData	*calibrationpoint
		= dynamic_cast<astro::guiding::CalibrationPointCallbackData *>(&*data);
	if (NULL != calibrationpoint) {
		// add the point to the calibration run
		astro::guiding::CalibrationStore	store(database);
		store.addPoint(calibrationid,
			calibrationpoint->calibrationpoint());
	}
	astro::guiding::GuiderCalibrationCallbackData	*calibration
		= dynamic_cast<astro::guiding::GuiderCalibrationCallbackData *>(&*data);
	if (NULL != calibration) {
		// add the calibration result to the database
		astro::guiding::CalibrationStore	store(database);
		store.updateCalibration(calibrationid,
			calibration->calibration());
	}

	// send calibration callbacks to the registered callbacks
	calibrationcallbacks(data);
}

} // namespace snowstar
