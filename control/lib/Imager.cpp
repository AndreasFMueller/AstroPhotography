/*
 * Imager.cpp -- Imager implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImager.h>
#include <AstroCalibration.h>
#include <AstroInterpolation.h>
#include <AstroDebug.h>

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
 * \brief Apply image correction
 */
void	Imager::operator()(ImagePtr image) {
	ImageRectangle	frame = image->getFrame();
	if ((_dark) && (_darksubtract)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "perform dark correction");
		DarkCorrector	corrector(_dark, frame);
		corrector(image);
	}
	if ((_flat) && (_flatdivide)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "perform flat correction");
		FlatCorrector	corrector(_flat, frame);
		corrector(image);
	}
	if ((_interpolate) && (_dark)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "interpolate bad pixels");
		Interpolator	interpolator(_dark, frame);
		interpolator(image);
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
ImagePtr	Imager::getImage() {
	ImagePtr	image = ccd()->getImage();
	this->operator()(image);
	return image;
}

} // namespace camera
} // namespace astro
