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
	return _state;
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
	_state.addCalibration();
	_guider->calibration(astro::convert(cal));
}

/**
 * \brief start calibrating
 */
void	Guider_impl::startCalibration(::CORBA::Float sensitivity) {
	_state.startCalibrating();
}

/**
 * \brief start guiding with the given interval
 */
void	Guider_impl::startGuiding(::CORBA::Float guidinginterval) {
	_state.startGuiding();
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
	_state.stopGuiding();
	_guider->stop();
}

ShortImage_ptr	Guider_impl::mostRecentImage() {
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
