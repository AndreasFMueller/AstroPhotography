/**
 * DarkCorrector.cpp -- perform dark correction
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCalibration.h>
#include <AstroFilter.h>
#include <PixelValue.h>
#include <limits>
#include <AstroDebug.h>
#include <stdexcept>
#include <vector>
#include <AstroFormat.h>

using namespace astro::image;
using namespace astro::image::filter;
using namespace astro::adapter;

namespace astro {
namespace calibration {

//////////////////////////////////////////////////////////////////////
// Type dark correctors
//
// Dark correction can be applied to any type of image, with varying
// primitive pixel types. These templates perform dark correction
// based on the various possible pixel types
//////////////////////////////////////////////////////////////////////
template<typename ImagePixelType, typename DarkPixelType>
void	dark_correct(Image<ImagePixelType>& image,
		const ConstImageAdapter<DarkPixelType>& dark,
		const int interpolation_distance = 0) {
	// first check that image sizes match
	if (image.size() != dark.getSize()) {
		std::string	msg = stringprintf("size: image %s != dark %s",
			image.size().toString().c_str(),
			dark.getSize().toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// correct all pixels
	int	bad_imagepixel_counter = 0;
	int	bad_darkpixel_counter = 0;
	for (int x = 0; x < image.size().width(); x++) {
		for (int y = 0; y < image.size().height(); y++) {
			ImagePixelType	ip = image.pixel(x, y);

			// skip NaN pixels, as we cannot do anything about them
			if (ip != ip) {
				bad_imagepixel_counter++;
				continue;
			}

			DarkPixelType	dp = dark.pixel(x, y);
			// turn off (make nan) pixels that are marked nan
			// in the dark
			if (dp != dp) {
				bad_darkpixel_counter++;
				// turn the off. If interpolation is enabled
				// they will be turned on again in the
				// interpolation step
				ip = 0;
			} else {
				if (ip > dp) {
					ip = ip - dp;
				} else {
					ip = 0;
				}
			}
			image.pixel(x, y) = ip;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "bad pixels: image %d, dark %d",
		bad_imagepixel_counter, bad_darkpixel_counter);

	// if interpolation_distance = 0, skip interpolation step
	if (0 == interpolation_distance) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "skipping interpolation step");
		return;
	}

	// interpolate neighbouring pixels
	for (int x = 0; x < image.size().width(); x++) {
		for (int y = 0; y < image.size().height(); y++) {
			// only work in this pixel of the dark pixel is a NaN
			DarkPixelType	dp = dark.pixel(x, y);
			if (dp == dp)
				continue;

			// find the everage of the neighboring pixels
			double	sum = 0;
			int	counter = 0;
			int	X = x + interpolation_distance;
			if (image.size().contains(X, y)) {
				sum += image.pixel(X, y);
				counter++;
			}
			X = x - interpolation_distance;
			if (image.size().contains(X, y)) {
				sum += image.pixel(X, y);
				counter++;
			}
			int	Y = y + interpolation_distance;
			if (image.size().contains(x, Y)) {
				sum += image.pixel(x, Y);
				counter++;
			}
			Y = y - interpolation_distance;
			if (image.size().contains(x, Y)) {
				sum += image.pixel(x, Y);
				counter++;
			}
			// if we have no neighbours, set the value to zero
			if (counter > 0) {
				sum = (1. / counter);
			}
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"interpolated value at (%d,%d) is %f (from %d)",
				x, y, sum, counter);
			image.pixel(x, y) = sum;
		}
	}
}

#define	dark_correct_for(T)						\
{									\
	Image<T>	*timage	= dynamic_cast<Image<T> *>(&*image);	\
	if (NULL != timage) {						\
		dark_correct(*timage, dark, interpolation_distance);	\
		return;							\
	}								\
}

template<typename DarkPixelType>
void	dark_correct_typed(ImagePtr image,
		const ConstImageAdapter<DarkPixelType>& dark,
		const int interpolation_distance = 0) {
	dark_correct_for(unsigned char);
	dark_correct_for(unsigned short);
	dark_correct_for(unsigned int);
	dark_correct_for(unsigned long);
	dark_correct_for(double);
	dark_correct_for(float);
	std::string	msg("dark correction only for primitive types");
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

//////////////////////////////////////////////////////////////////////
// DarkCorrector implementation
//////////////////////////////////////////////////////////////////////
DarkCorrector::DarkCorrector(const ImagePtr _dark,
	const ImageRectangle _rectangle)
	: Corrector(_dark, _rectangle) {
}

/**
 * \brief perform dark correction
 *
 * Subtract the dark image from the argument image. This is done in place,
 * as most of the time, the uncorrected image is no longer needed.
 * If a new image is required, first create the new image, then apply
 * the dark corrector in place.
 * \param image     image to dark correct
 * \param interpolation_distance
 */
void	DarkCorrector::operator()(ImagePtr image,
		const int interpolation_distance) const {
	Image<float>	*fp = dynamic_cast<Image<float> *>(&*calibrationimage);
	Image<double>	*dp = dynamic_cast<Image<double> *>(&*calibrationimage);
	if (NULL != fp) {
		WindowAdapter<float>	wa(*fp, rectangle);
		dark_correct_typed<float>(image, wa, interpolation_distance);
		return;
	}
	if (NULL != dp) {
		WindowAdapter<double>	wa(*dp, rectangle);
		dark_correct_typed<double>(image, wa, interpolation_distance);
		return;
	}
	std::string	msg("dark image must be of floating point type");
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

} // calibration
} // astro
