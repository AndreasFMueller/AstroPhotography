/*
 * Ccd_impl.cpp -- Corba Ccd implementation wrapper
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Ccd_impl.h"
#include "Cooler_impl.h"
#include "Image_impl.h"
#include <AstroExceptions.h>

using namespace astro::camera;

namespace Astro {

/**
 * \brief Query the current exposure status
 *
 * The various operations are only allowed in certain states, if these
 * precoditions are not met, the exposure related methods throw the BadState
 * exception.
 */
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

/**
 * \brief Start an exposure
 *
 * A new exposure can only be started, if the camera is in the idle state.
 * In any other state, this method throws a BadState exception;
 * \param exp	Exposure parameters
 */
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
	try {
		_ccd->startExposure(exposure);
	} catch (astro::BadParameter& bpx) {
		debug(LOG_ERR, DEBUG_LOG, 0, "bad parameter: %s", bpx.what());
		BadParameter	badparameter;
		badparameter.cause = bpx.what();
		throw badparameter;
	} catch (astro::camera::BadState& bsx) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"new exposure only state IDLE: %s", bsx.what());
		BadState	badstate;
		badstate.cause = bsx.what();
		throw badstate;
	}
}

/**
 * \brief Cancel an exposure
 *
 * Note that some cameras cannot cancel an exposure, in which case they
 * throw the NotImplemented exception.
 */
void	Ccd_impl::cancelExposure() {
	try {
		_ccd->cancelExposure();
	} catch (astro::NotImplemented& nix) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot cancel exposure: %s",
			nix.what());
		NotImplemented	notimplemented;
		notimplemented.cause = (const char *)nix.what();
		throw notimplemented;
	} catch (astro::camera::BadState &bsx) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cancel only state EXPOSING: %s", bsx.what());
		BadState	badstate;
		badstate.cause = bsx.what();
		throw badstate;
	}
}

/**
 * \brief Retrieve an Image from the CCD
 *
 * The CCD must be in state EXPOSED for this to be successful.
 */
Image_ptr	Ccd_impl::getImage() {
	if (!image) {
		try {
			image = _ccd->getImage();
		} catch (astro::camera::BadState& bsx) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"no image: %s", bsx.what());
			BadState	badstate;
			badstate.cause = bsx.what();
			throw badstate;
		}
	}
	Image_impl	*imageptr = new Image_impl(image);
	return imageptr->_this();
}

/**
 * \brief Get the Exposure
 *
 * The CCD must have performed an exposure previously for this method to
 * succeed.
 */
Astro::Exposure	Ccd_impl::getExposure() {
	try {
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
	} catch (astro::camera::BadState& bsx) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no exposure: %s", bsx.what());
		BadState	badstate;
		badstate.cause = bsx.what();
		throw badstate;
	}
}

/**
 * \brief Query whether this CCD has a gain setting
 */
::CORBA::Boolean	Ccd_impl::hasGain() {
	return _ccd->hasGain();
}

/**
 * \brief Query whether this CCD has a shutter
 */
::CORBA::Boolean	Ccd_impl::hasShutter() {
	return _ccd->hasShutter();
}

/**
 * \brief Query the shutter state
 */
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

/**
 * \brief Set the shutter state
 *
 * This method should normally not be used directly. Instead, the desired
 * shutter state during exposure should be requested in the Exposure
 * structure.
 */
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
	try {
		_ccd->setShutterState(shutterstate);
	} catch (astro::NotImplemented& nix) {
		debug(LOG_ERR, DEBUG_LOG, 0, "CCD cannot set shutter state");
		NotImplemented	notimplemented;
		notimplemented.cause = (const char *)"CCD cannot set shutter";
		throw notimplemented;
	}
}

/**
 * \brief Find out whether this CCD has a cooler.
 */
::CORBA::Boolean	Ccd_impl::hasCooler() {
	return _ccd->hasCooler();
}

/**
 * \brief Get the cooler
 */
Cooler_ptr	Ccd_impl::getCooler() {
	if (!_ccd->hasCooler()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "request for cooler on CCD that does not have one");
		NotImplemented	notimplemented;
		notimplemented.cause = (const char *)"CCD has no cooler";
		throw notimplemented;
	}
	CoolerPtr	cooler = _ccd->getCooler();

	// convert into a CORBA Cooler_ptr
	Cooler_impl	*coolerptr = new Cooler_impl(cooler);
	return coolerptr->_this();
}

} // namespace Astro
