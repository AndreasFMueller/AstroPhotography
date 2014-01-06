/*
 * Guider_impl.cpp -- implementation of the guider servant
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Guider_impl.h>
#include <Camera_impl.h>
#include <Ccd_impl.h>
#include <GuiderPort_impl.h>
#include <ServantBuilder.h>
#include <AstroCallback.h>
#include <AstroUtils.h>
#include <ImageObjectDirectory.h>
#include <Tracking.h>

extern astro::persistence::Database	database;

namespace Astro {

//////////////////////////////////////////////////////////////////////
// callback class for the images
//////////////////////////////////////////////////////////////////////
class GuiderImageCallback : public astro::callback::Callback {
	astro::Timer	timer;
	astro::image::ImagePtr	lastimage;
	Guider_impl&	_guider;
public:
	GuiderImageCallback(Guider_impl& guider);
	virtual astro::callback::CallbackDataPtr operator()(
		astro::callback::CallbackDataPtr data);
};

/**
 * \brief Create the callback
 *
 * We assume that this is equivalent to createing a new guider run
 */
GuiderImageCallback::GuiderImageCallback(Guider_impl& guider)
	: _guider(guider) {
}

/**
 * \brief process a new image
 */
astro::callback::CallbackDataPtr GuiderImageCallback::operator()(
	astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new image received");
	astro::guiding::GuiderNewImageCallbackData	*image
		= dynamic_cast<astro::guiding::GuiderNewImageCallbackData *>(&*data);
	if (NULL == image) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not image data");
		return data;
	}
	Astro::ImageSize	size = astro::convert(image->image()->size());

	// the access to the pixel array
	astro::image::Image<unsigned short>	*im
		= dynamic_cast<astro::image::Image<unsigned short> *>(
			&*image->image());
	if (NULL == im) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"only short images can be monitored");
		return data;
	}

	// prepare the imagedata
	::Astro::ShortSequence_var	imagedata = new Astro::ShortSequence();
	imagedata->length(size.width * size.height);

	// get the pixels
	for (unsigned int x = 0; x < size.width; x++) {
		for (unsigned int y = 0; y < size.height; y++) {
			imagedata[x + size.width * y] = im->pixel(x, y);
		}
	}

	// update all image monitors
	_guider.update(size, imagedata);

	return data;
}

//////////////////////////////////////////////////////////////////////
// callback class for tracking info
//////////////////////////////////////////////////////////////////////
class TrackingInfoCallback : public astro::callback::Callback {
	Guider_impl&	_guider;
	long	guidingrunid;
public:
	TrackingInfoCallback(Guider_impl& guider);
	virtual	astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data);
};

TrackingInfoCallback::TrackingInfoCallback(Guider_impl& guider)
	: _guider(guider) {
	astro::guiding::GuidingRun	guidingrun;
	guidingrun.camera = guider.getCameraName();
	guidingrun.ccdid = guider.getCcdid();
	guidingrun.guiderport = guider.getGuiderPortName();
	time(&guidingrun.whenstarted);
	astro::guiding::GuidingRunTable	guidingruntable(database);
	guidingrunid = guidingruntable.add(guidingrun);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new tracking run with id %d",
		guidingrunid);
}

/**
 * \brief Process a tracking info update
 */
astro::callback::CallbackDataPtr TrackingInfoCallback::operator()(
	astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new tracking info");
	astro::guiding::TrackingInfo	*trackinginfo
		= dynamic_cast<astro::guiding::TrackingInfo *>(&*data);

	// leave immediately, if there is not Tracking info
	if (NULL == trackinginfo) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"not a tracking info data");
		return data;
	}

	// update the guider, this will send the tracking info to the
	// registered clients
	_guider.update(astro::convert(*trackinginfo));

	// add an entry to the database
	astro::guiding::Tracking	tracking(0, guidingrunid, *trackinginfo);
	astro::guiding::TrackingTable	trackingtable(database);
	long	tid = trackingtable.add(tracking);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new tracking entry with id %ld", tid);

	return data;
}

//////////////////////////////////////////////////////////////////////
// Implementation of the Guider_impl class
//////////////////////////////////////////////////////////////////////
/**
 * \brief create a guider implementation object
 */
Guider_impl::Guider_impl(astro::guiding::GuiderPtr guider)
	: _guider(guider) {
	_point = _guider->ccd()->getInfo().getFrame().size().center();
	pthread_mutexattr_t	mattr;
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutex, &mattr);
}

/**
 * \brief Turn of the callbacks in the guider
 */
Guider_impl::~Guider_impl() {
}

/**
 * \brief retrieve the state of the state machine
 */
Guider::GuiderState	Guider_impl::getState() {
	return astro::convert(_guider->state());
}

/**
 * \brief Get a servant for the camera
 */
Camera_ptr	Guider_impl::getCamera() {
	astro::camera::CameraPtr	camera = _guider->camera();
	if (!camera) {
		throw BadState("no camera defined");
	}
	ServantBuilder<Camera, Camera_impl>	servant;
	return servant(camera);
}

/**
 * \brief Get a servant for the CCD
 */
Ccd_ptr	Guider_impl::getCcd() {
	astro::camera::CcdPtr	ccd = _guider->ccd();
	if (!ccd) {
		throw BadState("no ccd defined");
	}
	ServantBuilder<Ccd, Ccd_impl>	servant;
	return servant(ccd);
}

/**
 * \brief Get a servant for the Guiderport
 */
GuiderPort_ptr	Guider_impl::getGuiderPort() {
	astro::camera::GuiderPortPtr	guiderport = _guider->guiderport();
	if (!guiderport) {
		throw BadState("no guiderport defined");
	}
	ServantBuilder<GuiderPort, GuiderPort_impl>	servant;
	return servant(guiderport);
}

/**
 * \brief Configure the guider
 */
void	Guider_impl::setExposure(const ::Astro::Exposure& exposure) {
	astro::camera::Exposure	_exposure = astro::convert(exposure);
	_guider->exposure(_exposure);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure: %s", _exposure.toString().c_str());
}

/**
 * \brief set the star
 */
void	Guider_impl::setStar(const Astro::Point& star) {
	_point = astro::convert(star);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "star set to %s", _point.toString().c_str());
}

/**
 * \brief get the exposure used for the 
 */
Exposure	Guider_impl::getExposure() {
	return astro::convert(_guider->exposure());
}

/**
 * \brief Get the point on which the guide star should be locked
 */
Point	Guider_impl::getStar() {
	return astro::convert(_point);
}

/**
 * \brief Retrieve the calibration from the guider
 */
Guider::Calibration	Guider_impl::getCalibration() {
	return astro::convert(_guider->calibration());
}

/**
 * \brief Use the this calibration
 */
void	Guider_impl::useCalibration(const Astro::Guider::Calibration& cal) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"set calibration [ %.3f, %.3f, %.3f; %.3f, %.3f, %.3f ]",
		cal.coefficients[0], cal.coefficients[1], cal.coefficients[2],
		cal.coefficients[3], cal.coefficients[4], cal.coefficients[5]
	);
	_guider->calibration(astro::convert(cal));
}

/**
 * \brief start calibrating
 */
void	Guider_impl::startCalibration(::CORBA::Float focallength) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start calibration with focal length %f",
		focallength);
	// get the pixel size from the guider's ccd
	astro::camera::CcdInfo	info = _guider->ccd()->getInfo();
	float	pixelsize = (info.pixelwidth() + info.pixelheight()) / 2.;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixelsize: %fum", 1000000 * pixelsize);
	
	// get the tracker
	astro::guiding::TrackerPtr	tracker = getTracker();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracker constructed: %s",
		tracker->toString().c_str());

	// start calibration.
	_guider->startCalibration(tracker, focallength, pixelsize);
}

/**
 * \brief stop the calibration process
 */
void	Guider_impl::cancelCalibration() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cancel calibration");
	_guider->cancelCalibration();
}

/**
 * \brief wait for the calibration to complete
 */
bool	Guider_impl::waitCalibration(double timeout) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for calibration to to complete");
	return _guider->waitCalibration(timeout);
}

/**
 * \brief retrieve the progress info
 */
double	Guider_impl::calibrationProgress() {
	double	progress = _guider->calibrationProgress();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "check calibration progress: %f",
		progress);
	return progress;
}

/**
 * \brief start guiding with the given interval
 */
void	Guider_impl::startGuiding(::CORBA::Float guidinginterval) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start guiding with interval %f",
		guidinginterval);

	// create a set of callbacks
	_guider->newimagecallback = astro::callback::CallbackPtr(
		new GuiderImageCallback(*this));
	_guider->trackingcallback = astro::callback::CallbackPtr(
		new TrackingInfoCallback(*this));

	// Construct the tracker. The rectangle is a rectangle the size of the
	astro::guiding::TrackerPtr	tracker = getTracker();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracker constructed: %s",
		tracker->toString().c_str());

	// start calibration.
	_guider->startGuiding(tracker, guidinginterval);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guiding started");
}

/**
 * \brief get the guiding interval
 */
::CORBA::Float	Guider_impl::getGuidingInterval() {
	return 0;
}

/**
 * \brief stop the guiding process
 */
void	Guider_impl::stopGuiding() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop guiding");
	_guider->stopGuiding();

	// inform the monitors that we have stopped
	update_stop();

	// destroy the callbacks
	_guider->newimagecallback = NULL;
	_guider->trackingcallback = NULL;
}

/**
 * \brief get most recent image
 */
Image_ptr	Guider_impl::mostRecentImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve most recent image");
	// actuall retrieve the most recent image from the callback
	ImagePtr	image = _guider->mostRecentImage;
	if (!image) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "there is no most recent image");
		throw CORBA::OBJECT_NOT_EXIST();
	}

	// save the image in the image directory
        Astro::ImageObjectDirectory    directory;
        std::string     filename = directory.save(image);

        // activate this object
        return directory.getImage(filename);
}

Astro::TrackingInfo	Guider_impl::mostRecentTrackingInfo() {
	// verify that we really are guiding right now
	if (astro::guiding::guiding != _guider->state()) {
		throw BadState("not currently guiding");
	}

	// ok, we are guiding. Prepare a result structure
	Astro::TrackingInfo	result;
	// So we query the guider for the contents of this structure
	double	lastaction;
	astro::Point	offset;
	astro::Point	activation;
	_guider->lastAction(lastaction, offset, activation);
	result.timeago = astro::Timer::gettime() - lastaction;
	result.trackingoffset.x = offset.x();
	result.trackingoffset.y = offset.y();
	result.activation.x = activation.x();
	result.activation.y = activation.y();

	// that's it, we are read, return the structure
	return result;
}

Astro::GuiderDescriptor	*Guider_impl::getDescriptor() {
	Astro::GuiderDescriptor	*gd = new Astro::GuiderDescriptor();
	std::string	cameraname = _guider->camera()->name();
	gd->cameraname = CORBA::string_dup(cameraname.c_str());
	gd->ccdid = _guider->ccd()->getInfo().getId();
	std::string	guiderportname = _guider->guiderport()->name();
	gd->guiderportname = CORBA::string_dup(guiderportname.c_str());
	return gd;
}

astro::guiding::TrackerPtr	Guider_impl::getTracker() {
        astro::camera::Exposure exposure = _guider->exposure();
        debug(LOG_DEBUG, DEBUG_LOG, 0, "origin: %s", exposure.frame.origin().toString().c_str());
        debug(LOG_DEBUG, DEBUG_LOG, 0, "_point: %s", _point.toString().c_str());
        astro::Point    difference = _point - exposure.frame.origin();
	int	x = difference.x();
	int	y = difference.y();
        astro::image::ImagePoint        trackerstar(x, y);
        astro::image::ImageRectangle    trackerrectangle(exposure.frame.size());
        astro::guiding::TrackerPtr      tracker(
                new astro::guiding::StarTracker(trackerstar, trackerrectangle, 10));
	return tracker;
}

/**
 * \brief Register a tracking info monitor
 *
 * The Guider_impl class keeps the registered monitors in a map with the
 * monitor id as key. Registering a monitor means creating a new monitor id
 * never before used and putting the TrackingMonitor reference into the map
 * under this new id.
 *
 * Note the name caused by "register" being a reserved wird in C++.
 */
::CORBA::Long	Guider_impl::registerMonitor(::Astro::TrackingMonitor_ptr monitor) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "TrackingMonitor registration request: %p", this);
	pthread_mutex_lock(&mutex);
	::CORBA::Long	monitorid = 0;
	monitormap_t::iterator	i;
	for (i = monitors.begin(); i != monitors.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "checking id %d", i->first);
		if (i->first >= monitorid) {
			monitorid = i->first + 1;
		}
	}
	TrackingMonitor_var	monitorvar
		= Astro::TrackingMonitor::_duplicate(monitor);
	monitors.insert(std::make_pair(monitorid, monitorvar));
	pthread_mutex_unlock(&mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracking monitor registered as %ld",
		monitorid);
	return monitorid;
}

/**
 * \brief Unregister a monitor id
 *
 * \param monitorid	This is the monitor id returned by the register call
 */
void	Guider_impl::unregisterMonitor(::CORBA::Long monitorid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "unregister(%ld)", monitorid);
	pthread_mutex_lock(&mutex);
	if (monitors.find(monitorid) != monitors.end()) {
		try {
			monitors.erase(monitorid);
		} catch (...) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"failed to remove monitor %ld", monitorid);
		}
	} else {
		debug(LOG_ERR, DEBUG_LOG, 0, "monitor %ld does not exist",
			monitorid);
		throw CORBA::OBJECT_NOT_EXIST();
	}
	pthread_mutex_unlock(&mutex);
}

/**
 * \brief update distribution function
 *
 * This method sends the tracking info update to all registered tracking
 * monitors. However, if a monitor fails, it is removed and has to reregister.
 */
void	Guider_impl::update(const Astro::TrackingInfo& trackinginfo) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracking info update received");
	pthread_mutex_lock(&mutex);
	monitormap_t::iterator	i;
	std::vector< ::CORBA::Long>	badmonitors;
	// send the trackinginfo update to all monitors
	for (i = monitors.begin(); i != monitors.end(); i++) {
		try {
			i->second->update(trackinginfo);
		} catch (...) {
			badmonitors.push_back(i->first);
		}
	}
	// remove all monitors that are bad
	std::vector< ::CORBA::Long>::iterator	j;
	for (j = badmonitors.begin(); j != badmonitors.end(); j++) {
		try {
			monitors.erase(*j);
		} catch (...) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"error while removing monitor %ld", *j);
		}
	}
	pthread_mutex_unlock(&mutex);
}

/**
 * \brief Register a tracking image monitor
 *
 * The Guider_impl class keeps the registered image monitors in a map with the
 * monitor id as key. Registering a monitor means creating a new monitor id
 * never before used and putting the TrackingImageMonitor reference into the map
 * under this new id.
 *
 * Note the name caused by "register" being a reserved wird in C++.
 */
::CORBA::Long	Guider_impl::registerImageMonitor(::Astro::TrackingImageMonitor_ptr monitor) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "TrackingMonitor registration request: %p", this);
	pthread_mutex_lock(&mutex);
	::CORBA::Long	imagemonitorid = 0;
	imagemonitormap_t::iterator	i;
	for (i = imagemonitors.begin(); i != imagemonitors.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "checking id %d", i->first);
		if (i->first >= imagemonitorid) {
			imagemonitorid = i->first + 1;
		}
	}
	TrackingImageMonitor_var	imagemonitorvar
		= Astro::TrackingImageMonitor::_duplicate(monitor);
	imagemonitors.insert(std::make_pair(imagemonitorid, imagemonitorvar));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracking monitor registered as %ld",
		imagemonitorid);
	pthread_mutex_unlock(&mutex);
	return imagemonitorid;
}

/**
 * \brief Unregister a imagemonitor id
 *
 * \param monitorid	This is the image monitor id returned by the
 *			register call
 */
void	Guider_impl::unregisterImageMonitor(::CORBA::Long imagemonitorid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "unregisterImageMonitor(%ld)",
		imagemonitorid);
	pthread_mutex_lock(&mutex);
	if (imagemonitors.find(imagemonitorid) != imagemonitors.end()) {
		try {
			imagemonitors.erase(imagemonitorid);
		} catch (...) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"failed to remove image monitor %ld",
				imagemonitorid);
		}
	} else {
		debug(LOG_ERR, DEBUG_LOG, 0, "image monitor %ld does not exist",
			imagemonitorid);
		throw CORBA::OBJECT_NOT_EXIST();
	}
	pthread_mutex_unlock(&mutex);
}

/**
 * \brief update distribution function
 *
 * This method sends the tracking image update to all registered tracking
 * image monitors. However, if a monitor fails, it is removed and has to
 * reregister.
 */
void	Guider_impl::update(const ::Astro::ImageSize& size,
		const ::Astro::ShortSequence_var& imagedata) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracking image update received");
	pthread_mutex_lock(&mutex);
	imagemonitormap_t::iterator	i;
	std::vector< ::CORBA::Long>	badmonitors;
	// send the trackinginfo update to all monitors
	for (i = imagemonitors.begin(); i != imagemonitors.end(); i++) {
		try {
			i->second->update(size, imagedata);
		} catch (...) {
			badmonitors.push_back(i->first);
		}
	}
	// remove all monitors that are bad
	std::vector< ::CORBA::Long>::iterator	j;
	for (j = badmonitors.begin(); j != badmonitors.end(); j++) {
		try {
			imagemonitors.erase(*j);
		} catch (...) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"error while removing imagemonitor %ld", *j);
		}
	}
	pthread_mutex_unlock(&mutex);
}

/**
 * \brief Inform the clients that guiding has stopped
 */
void	Guider_impl::update_stop() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracking update stop received");
	pthread_mutex_lock(&mutex);
	monitormap_t::iterator	i;
	std::vector< ::CORBA::Long>	badmonitors;
	// send the trackinginfo update to all monitors
	for (i = monitors.begin(); i != monitors.end(); i++) {
		try {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "sending stop to %d",
				i->first);
			i->second->stop();
		} catch (...) {
			badmonitors.push_back(i->first);
		}
	}
	// remove all monitors that are bad
	std::vector< ::CORBA::Long>::iterator	j;
	for (j = badmonitors.begin(); j != badmonitors.end(); j++) {
		try {
			monitors.erase(*j);
		} catch (...) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"error while removing monitor %ld", *j);
		}
	}
	pthread_mutex_unlock(&mutex);
}

} // namespace Astro
