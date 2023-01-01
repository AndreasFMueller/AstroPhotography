/*
 * Imager.cpp -- Imager implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImager.h>
#include <AstroCalibration.h>
#include <AstroInterpolation.h>
#include <AstroDebug.h>
#include <AstroEvent.h>

using namespace astro::calibration;
using namespace astro::interpolation;

namespace astro {
namespace camera {

/**
 * \brief Create an Imager
 */
Imager::Imager(CcdPtr ccd) : _ccd(ccd) {
	_darksubtract = false;
	_flatdivide = false;
	_interpolate = false;
}

/**
 * \grief Destroy the imager
 */
Imager::~Imager() {
	if (_ccd) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "release controlling");
		_ccd->releaseControlling();
	}
}

void	Imager::dark(ImagePtr dark) {
	std::string	msg = stringprintf("install %s dark image in %s",
		dark->size().toString().c_str(),
		(_ccd) ? _ccd->name().toString().c_str() : "unknown");
	astro::event(EVENT_CLASS, astro::events::INFO,
		astro::events::Event::DEVICE, msg);
	_dark = dark;
}

void	Imager::flat(ImagePtr flat) {
	std::string	msg = stringprintf("install %s flat image in %s",
		flat->size().toString().c_str(),
		(_ccd) ? _ccd->name().toString().c_str() : "unknown");
	astro::event(EVENT_CLASS, astro::events::INFO,
		astro::events::Event::DEVICE, msg);
	_flat = flat;
}

/**
 * \brief Apply image correction
 */
void	Imager::operator()(ImagePtr image) {
	ImageRectangle	frame = image->getFrame();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "working on image %s",
			frame.toString().c_str());
	if ((_dark) && (_darksubtract)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "perform dark correction");
		DarkCorrector	corrector(_dark, frame);
		corrector(image);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "skipping dark correction");
	}
	if ((_flat) && (_flatdivide)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "perform flat correction");
		FlatCorrector	corrector(_flat, frame);
		corrector(image);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "skipping flat correction");
	}
	if ((_interpolate) && (_dark)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "interpolate bad pixels");
		Interpolator	interpolator(_dark, frame);
		interpolator(image);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "skipping interpolation");
	}
}

/**
 * \brief Start an exposure
 */
void	Imager::startExposure(const Exposure& exposure) {
	ccd()->startExposure(exposure);
}

/**
 * \brief Get a corrected image
 */
ImagePtr	Imager::getImage(bool raw) {
	// wait until there is an image
	if (!ccd()->wait()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no image available after wait");
		throw std::runtime_error("no image available");
	}
	ImagePtr	image = ccd()->getImage();
	if (raw) {
		return image;
	}
	this->operator()(image);
	return image;
}

bool	Imager::wait() {
	return ccd()->wait();
}

void	Imager::controlling(device::Device::controlState_t cs) {
	if (_ccd) {
		_ccd->controllingState(cs);
	}
}

void	Imager::release() {
	if (_ccd) {
		_ccd->releaseControlling();
	}
}

} // namespace camera
} // namespace astro
