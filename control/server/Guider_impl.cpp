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

namespace Astro {

//////////////////////////////////////////////////////////////////////
// callback class for the images
//////////////////////////////////////////////////////////////////////
class GuiderImageCallback : public astro::callback::Callback {
	astro::Timer	timer;
	astro::image::ImagePtr	lastimage;
public:
	GuiderImageCallback();
	virtual astro::callback::CallbackDataPtr operator()(
		astro::callback::CallbackDataPtr data);
};

GuiderImageCallback::GuiderImageCallback() {
}

astro::callback::CallbackDataPtr GuiderImageCallback::operator()(
	astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new image received");
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
	_guider->newimagecallback = astro::callback::CallbackPtr(new GuiderImageCallback());
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
}

Image_ptr	Guider_impl::mostRecentImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve most recent image");
	// actuall retrieve the most recent image from the callback
	ImagePtr	image = _guider->mostRecentImage;

        Astro::ImageObjectDirectory    directory;
        std::string     filename = directory.save(image);

        // activate this object
        return directory.getImage(filename);
}

Astro::Guider::TrackingInfo	Guider_impl::mostRecentTrackingInfo() {
	// verify that we really are guiding right now
	if (astro::guiding::guiding != _guider->state()) {
		throw BadState("not currently guiding");
	}

	// ok, we are guiding. Prepare a result structure
	Astro::Guider::TrackingInfo	result;
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
	// XXX implementation missing
	return NULL;
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

} // namespace Astro
