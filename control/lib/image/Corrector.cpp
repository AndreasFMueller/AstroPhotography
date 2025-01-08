/*
 * Corrector.cpp -- Corrector base class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCalibration.h>
#include <AstroFilter.h>
#include <stdexcept>
#include <AstroDebug.h>

using namespace astro::image;

namespace astro {
namespace calibration {

#define	countnans_typed(T)						\
{									\
	Image<T>	*image = dynamic_cast<Image<T> *>(&*calibrationimage);\
	if (image) {							\
		image::filter::CountNaNs<T, size_t>	c;		\
		adapter::WindowAdapter<T>    wa(*image, rectangle);	\
		_badpixels = c.filter(wa);				\
		debug(LOG_DEBUG, DEBUG_LOG, 0,				\
			"%d bad pixels found in %s image", _badpixels,	\
			calibrationimage->info().c_str());		\
		return;							\
	} else {							\
		debug(LOG_DEBUG, DEBUG_LOG, 0,	"calibration image "	\
			"pixel type is not %s", #T);			\
	}								\
}

/**
 * \brief Construct a corrector
 *
 * The constructor finds out whether the calibration image has pixels
 * of floating point type and determines the number of NaNs present
 * in the image. This can then be used to decide whether it is necessary
 * to perform any interpolations
 *
 * \param _calibrationimage	the calibration image to use
 * \param _rectangle		the rectangle to use for calibrating images
 */
Corrector::Corrector(const ImagePtr _calibrationimage,
	const ImageRectangle _rectangle)
	: calibrationimage(_calibrationimage), rectangle(_rectangle) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Corrector for %s-image in rectangle %s",
		_calibrationimage->info().c_str(),
		_rectangle.toString().c_str());

	// first compute the rectangle from which to take the dark data
	if (rectangle == ImageRectangle()) {
		// if the rectangle is the default (empty) rectangle, take
		// the calibration image rectangle instead
		rectangle = ImageRectangle(ImagePoint(),
			 calibrationimage->size());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using dark rectangle %s",
		rectangle.toString().c_str());

	// We want dark images to be of float or double type
	countnans_typed(float)
	countnans_typed(double)

	// if we get to this point, then the dark is not of float type
	std::string	msg = astro::stringprintf("calibration image must be "
		"of floating point type, but is %s",
		_calibrationimage->info().c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Interpolation for bad pixels in the calibration frame
 *
 * \param image				the image to calibrate/interpolate
 * \param interpolation_distance	the grid size for the interpolation
 */
void	Corrector::operator()(astro::image::ImagePtr image,
		const int interpolation_distance) const {
	if (0 == interpolation_distance)
		return;
	if (0 == badpixels())
		return;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "perform interpolation");
	CalibrationInterpolation	ci(interpolation_distance > 0);
	ci(image, calibrationimage);
}

} // namespace calibration
} // namespace astro 
