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
#include <AstroFormat.h>
#include <ImageObjectDirectory.h>
#include <TrackingPersistence.h>
#include <Conversions.h>
#include <GuiderImageCallback.h>
#include <TrackingInfoCallback.h>
#include <CalibrationPointCallback.h>
#include <GuiderFactory_impl.h>

extern astro::persistence::Database	database;

namespace Astro {

/**
 * \brief create a guider implementation object
 */
Guider_impl::Guider_impl(astro::guiding::GuiderPtr guider)
	: _guider(guider) {
	_point = _guider->ccd()->getInfo().getFrame().size().center();
	guidingrunid = -1;
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
 * \brief Retrieve the descriptor of this guider
 */
Astro::GuiderDescriptor	*Guider_impl::getDescriptor() {
	Astro::GuiderDescriptor	*gd = new Astro::GuiderDescriptor();
	std::string	cameraname = _guider->camera()->name();
	gd->cameraname = CORBA::string_dup(cameraname.c_str());
	gd->ccdid = _guider->ccd()->getInfo().getId();
	std::string	guiderportname = _guider->guiderport()->name();
	gd->guiderportname = CORBA::string_dup(guiderportname.c_str());
	return gd;
}

/**
 * \brief Get a reference to the tracker
 */
astro::guiding::TrackerPtr	Guider_impl::getTracker() {
        astro::camera::Exposure exposure = _guider->exposure();
        debug(LOG_DEBUG, DEBUG_LOG, 0, "origin: %s",
		exposure.frame.origin().toString().c_str());
        debug(LOG_DEBUG, DEBUG_LOG, 0, "_point: %s", _point.toString().c_str());
        astro::Point    difference = _point - exposure.frame.origin();
	int	x = difference.x();
	int	y = difference.y();
        astro::image::ImagePoint        trackerstar(x, y);
        astro::image::ImageRectangle    trackerrectangle(exposure.frame.size());
        astro::guiding::TrackerPtr      tracker(
                new astro::guiding::StarTracker(trackerstar,
			trackerrectangle, 10));
	return tracker;
}

} // namespace Astro
