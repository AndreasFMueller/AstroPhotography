/*
 * Guider_impl.cpp -- implementation of the guider servant
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Guider_impl.h>

namespace Astro {

Guider::GuiderState	Guider_impl::getState() {
	return ::Astro::Guider::GUIDER_UNCONFIGURED;
}

Camera_ptr	Guider_impl::getCamera() {
	return NULL;
}

Ccd_ptr	Guider_impl::getCcd() {
	return NULL;
}

GuiderPort_ptr	Guider_impl::getGuiderPort() {
	return NULL;
}

void	Guider_impl::setupGuider(const ::Astro::ImageRectangle& rectangle, const ::Astro::Guider::Point& star, ::CORBA::Float exposuretime) {
}

Astro::ImageRectangle Guider_impl::selectedArea() {
	Astro::ImageRectangle	rectangle;
	return rectangle;
}

Astro::Guider::Point	Guider_impl::selectedPoint() {
	Astro::Guider::Point	result;
	return result;
}

::CORBA::Float	Guider_impl::exposureTime() {
	return 0;
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

Astro::Guider::Point	Guider_impl::mostRecentOffset() {
	Astro::Guider::Point	point;
	return point;
}

::CORBA::Float	Guider_impl::mostRecentDelay() {
	return 0;
}


Astro::Guider::GuiderAction	Guider_impl::mostRecentAction() {
	::Astro::Guider::GuiderAction	action;
	return  action;
}

} // namespace Astro
