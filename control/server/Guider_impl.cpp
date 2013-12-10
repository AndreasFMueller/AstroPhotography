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

namespace Astro {

/**
 * \brief create a guider implementation object
 */
Guider_impl::Guider_impl(astro::guiding::GuiderPtr guider)
	: _guider(guider) {
	_point = _guider->ccd()->getInfo().getFrame().size().center();
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
	_guider->exposure(astro::convert(exposure));
}

/**
 * \brief set the star
 */
void	Guider_impl::setStar(const Astro::Point& star) {
	_point = astro::convert(star);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixelsize: %f", pixelsize);
	
	// construct the tracker
	astro::camera::Exposure	exposure = _guider->exposure();
	astro::guiding::TrackerPtr	tracker(
		new astro::guiding::StarTracker(_point, exposure.frame, 10));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracker constructed");

	// start calibration
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
	// XXX actually do the guiding
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
	_guider->stop();
}

ShortImage_ptr	Guider_impl::mostRecentImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve most recent image");
	// XXX actuall retrieve the most recent image
	return NULL;
}

Astro::Point	Guider_impl::mostRecentOffset() {
	Astro::Point	point;
	return point;
}

::CORBA::Float	Guider_impl::mostRecentDelay() {
	return 0;
}


Astro::Guider::GuiderAction	Guider_impl::mostRecentAction() {
	::Astro::Guider::GuiderAction	action;
	return  action;
}

Astro::GuiderDescriptor	*Guider_impl::getDescriptor() {
	return NULL;
}

} // namespace Astro
