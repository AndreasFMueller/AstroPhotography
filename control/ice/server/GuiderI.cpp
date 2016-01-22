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
#include <AstroConfig.h>
#include <TrackingPersistence.h>
#include <TrackingStore.h>
#include "CalibrationSource.h"

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
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not a tracking info object");
		return;
	}

	// tracking 
	TrackingPoint	trackingpoint = convert(*trackinginfo);
	p->update(trackingpoint);
}

/**
 * \brief Function to copy image pixels to the SimpleImage structure
 */
template<typename pixel>
void	copy_image(const astro::image::Image<pixel> *source,
		SimpleImage& target, double scale) {
	for (int y = 0; y < target.size.height; y++) {
		for (int x = 0; x < target.size.width; x++) {
			unsigned short	value = scale * source->pixel(x, y);
			target.imagedata.push_back(value);
		}
	}
}

#define copypixels(pixel, scale, source, target)		\
do {								\
	astro::image::Image<pixel>	*im			\
		= dynamic_cast<astro::image::Image<pixel> *>(	\
			&*source);				\
	if (NULL != im) {					\
		copy_image(im, target, scale);			\
	}							\
} while (0)

/**
 * \brief calback adapter for Tracking monitor
 */
template<>
void	callback_adapter<ImageMonitorPrx>(ImageMonitorPrx& p,
		const astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image callback called");
	// first check whether we really got an image
	astro::callback::ImageCallbackData	*imageptr
		= dynamic_cast<astro::callback::ImageCallbackData *>(&*data);
	if (NULL == imageptr) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "ignoring non-ImageCallbackData");
		return;
	}

	// source image
	astro::image::ImagePtr	source = imageptr->image();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback image has size %s",
		source->size().toString().c_str());

	// convert the image so that it is understood by the
	// ImageMonitor proxy
	SimpleImage	target;
	target.size = convert(source->size());

	// copy pixels to the target structure
	copypixels(unsigned short, 1, source, target);
	copypixels(unsigned char, 256, source, target);
	copypixels(unsigned long, (1. / 65536), source, target);
	copypixels(double, 1, source, target);
	copypixels(float, 1, source, target);
	
	if ((0 == target.size.width) && (0 == target.size.height)) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"don't know how to handle non short images");
		return;
	}

	// now that the image has been created, send it to the callback
	p->update(target);
}

/**
 * \brief calback adapter for Calibration monitor
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
			= convert(calibrationpoint->data());
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
	// set point to an invalid value to allows to detect that it has not been set
	_point.x = -1;
	_point.y = -1;
	// default tracker is star
	_method = TrackerSTAR;

	// callback stuff
	debug(LOG_DEBUG, DEBUG_LOG, 0, "installing calibration callbacks");
	GuiderICalibrationCallback	*ccallback
		= new GuiderICalibrationCallback(*this);
	guider->addCalibrationCallback(astro::callback::CallbackPtr(ccallback));
	guider->addGuidercalibrationCallback(astro::callback::CallbackPtr(ccallback));

	GuiderIImageCallback	*icallback = new GuiderIImageCallback(*this);
	guider->addImageCallback(astro::callback::CallbackPtr(icallback));

	debug(LOG_DEBUG, DEBUG_LOG, 0, "installing tracking callbacks");
	GuiderITrackingCallback	*tcallback = new GuiderITrackingCallback(*this);
	guider->addTrackingCallback(astro::callback::CallbackPtr(tcallback));
}

GuiderI::~GuiderI() {
}

GuiderState GuiderI::getState(const Ice::Current& /* current */) {
	return convert(guider->state());
}

CcdPrx GuiderI::getCcd(const Ice::Current& current) {
	std::string	ccdname = guider->getDescriptor().ccd();
	return CcdI::createProxy(ccdname, current);
}

GuiderPortPrx GuiderI::getGuiderPort(const Ice::Current& current) {
	std::string	name = guider->getDescriptor().guiderport();
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

TrackerMethod	GuiderI::getTrackerMethod(const Ice::Current& /* current */) {
	return _method;
}

void	GuiderI::setTrackerMethod(TrackerMethod method, const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using method: %s",
		(method == TrackerUNDEFINED) ? "undefined" : (
			(method == TrackerSTAR) ? "star" : (
				(method == TrackerPHASE) ? "phase" : "diff")));
	_method = method;
}

void GuiderI::useCalibration(Ice::Int calid,
	const Ice::Current& /* current */) {
	// retrieve guider data from the database
	astro::guiding::CalibrationStore	store(database);
	if (store.contains(calid, astro::guiding::BasicCalibration::GP)) {
		guider->guiderPortDevice->calibrationid(calid);
	}

	if (store.contains(calid, astro::guiding::BasicCalibration::AO)) {
		guider->adaptiveOpticsDevice->calibrationid(calid);
	}
}

Calibration GuiderI::getCalibration(CalibrationType calibrationtype, const Ice::Current& /* current */) {
	CalibrationSource	source(database);
	switch (calibrationtype) {
	case CalibrationTypeGuiderPort:
		return source.get(guider->guiderPortDevice->calibrationid());
	case CalibrationTypeAdaptiveOptics:
		return source.get(guider->adaptiveOpticsDevice->calibrationid());
	}
}

/**
 * \brief Start a calibration for a given focal length
 *
 * The focal length is the only piece of information that we can not
 * get from anywhere else, so it has to be specified
 */
Ice::Int GuiderI::startCalibration(CalibrationType caltype, const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start calibration, type = %s",
		calibrationtype2string(caltype).c_str());

	// construct a tracker
	astro::guiding::TrackerPtr	tracker = getTracker();

	// start the calibration
	switch (caltype) {
	case CalibrationTypeGuiderPort:
		return guider->startCalibration(astro::guiding::BasicCalibration::GP, tracker);
	case CalibrationTypeAdaptiveOptics:
		return guider->startCalibration(astro::guiding::BasicCalibration::AO, tracker);
	}
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
	guider->guiderPortDevice->cancelCalibration();
}

/**
 * \brief Wait for the calibration to complete
 */
bool GuiderI::waitCalibration(Ice::Double timeout,
	const Ice::Current& /* current */) {
	return guider->guiderPortDevice->waitCalibration(timeout);
}

/**
 * \brief build a tracker
 */
astro::guiding::TrackerPtr	 GuiderI::getTracker() {
	// first we must make sure the data we have is consistent
	astro::camera::Exposure	exposure = guider->exposure();
	if ((exposure.frame().size().width() <= 0) ||
		(exposure.frame().size().height() <= 0)) {
		// get the frame from the ccd
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using ccd frame");
		exposure.frame(guider->imager().ccd()->getInfo().getFrame());
		guider->exposure(exposure);
	}
	if ((_point.x < 0) || (_point.y < 0)) {
		astro::image::ImagePoint	c = exposure.frame().center();
		_point.x = c.x();
		_point.y = c.y();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using ccd center (%.1f,%.1f) as star",
			_point.x, _point.y);
	}

	switch (_method) {
	case TrackerUNDEFINED:
	case TrackerSTAR:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "construct a star tracker");
		return guider->getTracker(convert(_point));
		break;
	case TrackerPHASE:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "construct a phase tracker");
		return guider->getPhaseTracker();
		break;
	case TrackerDIFFPHASE:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "construct a diff tracker");
		return guider->getDiffPhaseTracker();
		break;
	}
}

/**
 * \brief Start guiding
 */
void GuiderI::startGuiding(Ice::Float guidinginterval,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start guiding with interval %.1f",
		guidinginterval);
	// construct a tracker
	astro::guiding::TrackerPtr	tracker = getTracker();

	// install a callback in the guider

	GuiderIImageCallback	*icallback = new GuiderIImageCallback(*this);
	guider->addImageCallback(astro::callback::CallbackPtr(icallback));

	// start guiding
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start guiding");
	guider->startGuiding(tracker, guidinginterval);
}

Ice::Float GuiderI::getGuidingInterval(const Ice::Current& /* current */) {
	return guider->getInterval();
}

void GuiderI::stopGuiding(const Ice::Current& /* current */) {
	guider->stopGuiding();

	// send the clients that guiding was stopped
	trackingcallbacks.stop();
	//imagecallbacks.stop();

	// remove the callback
	//guider->trackingcallback(NULL);
}

ImagePrx GuiderI::mostRecentImage(const Ice::Current& current) {
	// retrieve image
	astro::image::ImagePtr	image = guider->mostRecentImage();
	if (!image) {
		throw NotFound("no image available");
	}

	// store image in image directory
	std::string	filename = imagedirectory.save(image);

	// return a proxy for the image
	return getImage(filename, image->bytesPerPixel(), current);
}

TrackingPoint GuiderI::mostRecentTrackingPoint(const Ice::Current& /* current */) {
	if (astro::guiding::Guide::guiding != guider->state()) {
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get tracking history %d", id);
	astro::guiding::TrackingStore	store(database);
	return convert(store.get(id));
}

/**
 * \brief Register a callback for the calibration process
 */
void	GuiderI::registerCalibrationMonitor(const Ice::Identity& calibrationcallback, const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration callback registered");
	try {
		calibrationcallbacks.registerCallback(calibrationcallback,
			current);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot register calibration callback: %s %s",
			astro::demangle(typeid(x).name()).c_str(), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot register calibration callback for unknown reason");
	}
}

/**
 * \brief Unregister a callback for the calibration process
 */
void	GuiderI::unregisterCalibrationMonitor(const Ice::Identity& calibrationcallback, const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration callback unregistered");
	calibrationcallbacks.unregisterCallback(calibrationcallback, current);
}

/**
 * \brief Register a callback for images taken during the process
 */
void    GuiderI::registerImageMonitor(const Ice::Identity& imagecallback,
		const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "register an image callback");
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "register tracking callback");
	try {
		trackingcallbacks.registerCallback(trackingcallback, current);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot register tracking callback: %s %s",
			astro::demangle(typeid(x).name()).c_str(), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot register tracking callback for unknown reason");
	}
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
	// tell the clients that data has changed
	trackingcallbacks(data);
}

/**
 * \brief Handle a new image from the tracking process
 */
void	GuiderI::trackingImageUpdate(const astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "trackingImageUpdate called");

	if (imagerepo) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "send image to repository %s",
			_repositoryname.c_str());
		astro::callback::ImageCallbackData	*imageptr
			= dynamic_cast<astro::callback::ImageCallbackData *>(&*data);
		if (NULL == imageptr) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"ignoring non-ImageCallbackData");
		} else {
			// save the image in the repository
			imagerepo->save(imageptr->image());
		}
	}

	// sending data to all registered callbacks
	imagecallbacks(data);
}

/**
 * \brief Handle an update from the calibration process
 */
void	GuiderI::calibrationUpdate(const astro::callback::CallbackDataPtr data) {

	// send calibration callbacks to the registered callbacks
	calibrationcallbacks(data);
}

/**
 * \brief Handle the summary retrieval method
 */
TrackingSummary	GuiderI::getTrackingSummary(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calling for tracking summary");
	if (astro::guiding::Guide::guiding != guider->state()) {
		BadState	exception;
		exception.cause = astro::stringprintf("guider is not wrong "
			"state %s", astro::guiding::Guide::state2string(
			guider->state()).c_str());
		throw exception;
	}
	return convert(guider->summary());
}

/**
 * \brief retrieve the name of the current repository
 */
std::string	GuiderI::getRepositoryName(const Ice::Current& /* current */) {
	return _repositoryname;
}

/**
 * \brief activate sending images to the repository
 */
void	GuiderI::setRepositoryName(const std::string& reponame,
		const Ice::Current& /* current */) {
	// special case: zero length repo name means turn of storing images
	// in the repository
	if (0 == reponame.size()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "removing repository '%s'",
			_repositoryname.c_str());
		_repositoryname = reponame;
		imagerepo.reset();
		return;
	}

	// check that this repository actually exists
	astro::config::ImageRepoConfigurationPtr	config
		= astro::config::ImageRepoConfiguration::get();
	if (!config->exists(reponame)) {
		// throw an error
		NotFound	exception;
		exception.cause = astro::stringprintf("repository %s not found",
			reponame.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", exception.cause.c_str());
		throw exception;
	}
	imagerepo = config->repo(reponame);
	_repositoryname = reponame;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using repository %s",
		_repositoryname.c_str());
}

} // namespace snowstar
