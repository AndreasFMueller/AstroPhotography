/*
 * Ccd_impl.cpp -- Corba Ccd implementation wrapper
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Ccd_impl.h"
#include "Cooler_impl.h"
#include "Image_impl.h"
#include <AstroExceptions.h>
#include <AstroFilterfunc.h>
#include <Conversions.h>
#include <ImageObjectDirectory.h>
#include <ServantBuilder.h>

using namespace astro::camera;

namespace Astro {

Ccd_impl::Ccd_impl(astro::camera::CcdPtr ccd) : _ccd(ccd) {
	laststart = 0;
}

char	*Ccd_impl::getName() {
	std::string	name = _ccd->name();
	return CORBA::string_dup(name.c_str());
}

/**
 * \brief Retrieve time since last image start
 */
CORBA::Long	Ccd_impl::lastExposureStart() {
	time_t	now = time(NULL);
	return now - laststart;
}

/**
 * \brief Ccd
 *
 * get the information from the server
 */
CcdInfo	*Ccd_impl::getInfo() {
	CcdInfo	*result = astro::convert(_ccd->getInfo());
	return result;
}

/**
 * \brief Query the current exposure status
 *
 * The various operations are only allowed in certain states, if these
 * precoditions are not met, the exposure related methods throw the BadState
 * exception.
 */
ExposureState	Ccd_impl::exposureStatus() {
	return astro::convert(_ccd->exposureStatus());
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
	astro::camera::Exposure	exposure = astro::convert(exp);
	try {
		_ccd->startExposure(exposure);
		laststart = time(NULL);
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
			"cancel only in state EXPOSING or EXPOSED: %s", bsx.what());
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
	// first make sure we have an image
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

	// save the image
	ImageObjectDirectory	directory;
	std::string	filename = directory.save(image);

	// activate this object
	return directory.getImage(filename);
}

/**
 * \brief Get the Exposure
 *
 * The CCD must have performed an exposure previously for this method to
 * succeed.
 */
Astro::Exposure	Ccd_impl::getExposure() {
	try {
		Astro::Exposure	exp = astro::convert(_ccd->getExposure());
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
	return astro::convert(_ccd->getShutterState());
}

/**
 * \brief Set the shutter state
 *
 * This method should normally not be used directly. Instead, the desired
 * shutter state during exposure should be requested in the Exposure
 * structure.
 */
void	Ccd_impl::setShutterState(ShutterState state) {
	astro::camera::Shutter::state	shutterstate = astro::convert(state);
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
		debug(LOG_ERR, DEBUG_LOG, 0,
			"request for cooler on CCD that does not have one");
		NotImplemented	notimplemented;
		notimplemented.cause = (const char *)"CCD has no cooler";
		throw notimplemented;
	}
	CoolerPtr	cooler = _ccd->getCooler();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a cooler: %s",
		cooler->name().toString().c_str());

	// Use the ServantBuilder to create a servant
	ServantBuilder<Cooler, Cooler_impl>	servantbuilder;
	return servantbuilder(cooler);
}

} // namespace Astro
