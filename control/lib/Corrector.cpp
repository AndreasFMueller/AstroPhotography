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

/**
 * \brief Construct a corrector
 */
Corrector::Corrector(const ImagePtr& _calibrationimage,
	const ImageRectangle& _rectangle)
	: calibrationimage(_calibrationimage), rectangle(_rectangle) {
	// first compute the rectangle from which to take the dark data
	if (rectangle == ImageRectangle()) {
		rectangle = ImageRectangle(ImagePoint(),
			 calibrationimage->size());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using dark rectangle %s",
		rectangle.toString().c_str());

	// We want dark images to be of float or double type
	Image<float>	*fp = dynamic_cast<Image<float> *>(&*calibrationimage);
	Image<double>	*dp = dynamic_cast<Image<double> *>(&*calibrationimage);
	if ((NULL != fp) || (NULL != dp)) {
		return;
	}

	// if we get to this point, then the dark is not of float type
	std::string	msg("dark image must be of floating point type");
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

} // namespace calibration
} // namespace astro 
