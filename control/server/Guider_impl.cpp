/* * Guider_impl.cpp -- implementation of the guider servant
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Guider_impl.h>
#include <Camera_impl.h>
#include <Ccd_impl.h>
#include <GuiderPort_impl.h>
#include <ServantBuilder.h>

namespace Astro {

Guider_impl::Guider_impl(astro::guiding::GuiderPtr guider)
	: _state(Astro::Guider::GUIDER_UNCONFIGURED), _guider(guider) {
}

Guider::GuiderState	Guider_impl::getState() {
	return _state;
}

Camera_ptr	Guider_impl::getCamera() {
	astro::camera::CameraPtr	camera = _guider->camera();
	if (!camera) {
		throw BadState("no camera defined");
	}
	ServantBuilder<Camera, Camera_impl>	servant;
	return servant(camera);
}

Ccd_ptr	Guider_impl::getCcd() {
	astro::camera::CcdPtr	ccd = _guider->ccd();
	if (!ccd) {
		throw BadState("no ccd defined");
	}
	ServantBuilder<Ccd, Ccd_impl>	servant;
	return servant(ccd);
}

GuiderPort_ptr	Guider_impl::getGuiderPort() {
	astro::camera::GuiderPortPtr	guiderport = _guider->guiderport();
	if (!guiderport) {
		throw BadState("no guiderport defined");
	}
	ServantBuilder<GuiderPort, GuiderPort_impl>	servant;
	return servant(guiderport);
}

void	Guider_impl::setupGuider(const ::Astro::Exposure& exposure,
		const ::Astro::Point& star) {
	
}

Astro::Exposure	Guider_impl::getExposure() {
	Astro::Exposure	result;
	return result;
}

Astro::Point	Guider_impl::selectedPoint() {
	Astro::Point	result;
	return result;
}

Astro::Guider::Calibration	*Guider_impl::getCalibration() {
	return NULL;
}

void	Guider_impl::useCalibration(const Astro::Guider::Calibration& cal) {
}

void	Guider_impl::startCalibration(::CORBA::Float sensitivity) {
}

void	Guider_impl::startGuiding(::CORBA::Float guidinginterval) {
}

::CORBA::Float	Guider_impl::getGuidingInterval() {
	return 0;
}

void	Guider_impl::stopGuiding() {
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
