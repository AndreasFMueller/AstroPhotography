/*
 * Ccd_impl.cpp -- Corba Ccd implementation wrapper
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Ccd_impl.h"
#include "Cooler_impl.h"
#include "Image_impl.h"

using namespace astro::camera;

namespace Astro {

void	Ccd_impl::startExposure(const Exposure& exp) {
	image.reset();
	astro::camera::Exposure	exposure(
		astro::image::ImageRectangle(
			astro::image::ImagePoint(exp.frame.origin.x, exp.frame.origin.y),
			astro::image::ImageSize(exp.frame.size.width, exp.frame.size.height)
		),
		exp.exposuretime
	);
	exposure.gain = exp.gain;
	if (exp.limit > 0) {
		exposure.limit = exp.limit;
	}
	exposure.mode = Binning(exp.mode.x, exp.mode.y);
	switch (exp.shutter) {
	case Astro::SHUTTER_CLOSED:
		exposure.shutter = astro::camera::SHUTTER_CLOSED;
		break;
	case Astro::SHUTTER_OPEN:
		exposure.shutter = astro::camera::SHUTTER_OPEN;
		break;
	}
	_ccd->startExposure(exposure);
}

ExposureState	Ccd_impl::exposureStatus() {
	astro::camera::Exposure::State	state = _ccd->exposureStatus();
	switch (state) {
	case astro::camera::Exposure::idle:
		return EXPOSURE_IDLE;
	case astro::camera::Exposure::exposing:
		return EXPOSURE_EXPOSING;
	case astro::camera::Exposure::exposed:
		return EXPOSURE_EXPOSED;
	case astro::camera::Exposure::cancelling:
		return EXPOSURE_CANCELLING;
	}
}

void	Ccd_impl::cancelExposure() {
	_ccd->cancelExposure();
}

Image_ptr	Ccd_impl::getImage() {
	if (!image) {
		image = _ccd->getImage();
	}
	Image_impl	*imageptr = new Image_impl(image);
	return imageptr->_this();
}

Astro::Exposure	Ccd_impl::getExposure() {
	astro::camera::Exposure	exposure = _ccd->getExposure();
	Astro::Exposure	exp;
	exp.frame.size.width = exposure.frame.size().width();
	exp.frame.size.height = exposure.frame.size().height();
	exp.frame.origin.x = exposure.frame.origin().x();
	exp.frame.origin.y = exposure.frame.origin().y();
	exp.exposuretime = exposure.exposuretime;
	exp.gain = exposure.gain;
	exp.limit = exposure.limit;
	switch (exposure.shutter) {
	case astro::camera::SHUTTER_OPEN:
		exp.shutter = Astro::SHUTTER_OPEN;
	case astro::camera::SHUTTER_CLOSED:
		exp.shutter = Astro::SHUTTER_CLOSED;
	}
	exp.mode.x = exposure.mode.getX();
	exp.mode.y = exposure.mode.getY();
	return exp;
}

::CORBA::Boolean	Ccd_impl::hasGain() {
	return _ccd->hasGain();
}

::CORBA::Boolean	Ccd_impl::hasShutter() {
	return _ccd->hasShutter();
}

ShutterState	Ccd_impl::getShutterState() {
	astro::camera::shutter_state	shutterstate = _ccd->getShutterState();
	switch (shutterstate) {
	case astro::camera::SHUTTER_OPEN:
		return Astro::SHUTTER_OPEN;
		break;
	case astro::camera::SHUTTER_CLOSED:
		return Astro::SHUTTER_CLOSED;
		break;
	}
	// XXX should not happen
}

void	Ccd_impl::setShutterState(ShutterState state) {
	astro::camera::shutter_state	shutterstate;
	switch (state) {
	case Astro::SHUTTER_OPEN:
		shutterstate = astro::camera::SHUTTER_OPEN;
		break;
	case Astro::SHUTTER_CLOSED:
		shutterstate = astro::camera::SHUTTER_CLOSED;
		break;
	}
	_ccd->setShutterState(shutterstate);
}

::CORBA::Boolean	Ccd_impl::hasCooler() {
	return _ccd->hasCooler();
}

Cooler_ptr	Ccd_impl::getCooler() {
	if (_ccd->hasCooler()) {
		// XXX bad things happen
	}
	CoolerPtr	cooler = _ccd->getCooler();

	// convert into a CORBA Cooler_ptr
	Cooler_impl	*coolerptr = new Cooler_impl(cooler);
	return coolerptr->_this();
}

} // namespace Astro
