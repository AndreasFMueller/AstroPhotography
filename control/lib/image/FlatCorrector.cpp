/**
 * FlatCorrector.cpp -- perform flat correction
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

namespace astro {
namespace calibration {

//////////////////////////////////////////////////////////////////////
// Type flat correctors
//
// Flat correction can be applied to any type of image, with varying
// primitive pixel types. These templates perform flat correction
// based on the various possible pixel types
//////////////////////////////////////////////////////////////////////
template<typename ImagePixelType, typename FlatPixelType>
void	flat_correct(Image<ImagePixelType>& image,
		const Image<FlatPixelType>& flat,
		const int interpolation_distance = 0) {
	ImagePixelType	max = std::numeric_limits<ImagePixelType>::max();

	// first check that image sizes match
	if (image.size() != flat.size()) {
		std::string	msg = stringprintf("size: image %s != flat %s",
			image.size().toString().c_str(),
			flat.size().toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// correct all pixels where the flat is not nan
	int	bad_imagepixel_counter = 0;
	int	bad_flatpixel_counter = 0;
	for (size_t offset = 0; offset < image.size().getPixels(); offset++) {
		ImagePixelType	ip = image.pixels[offset];
		// skip NaN pixels in the image, as we cannot do anything 
		// about them. Normally, such pixels do not exist
		if (ip != ip) {
			bad_imagepixel_counter++;
			continue;
		}
		FlatPixelType	dp = flat.pixels[offset];
		// leave alone pixels that are marked nan in the flat
		if (dp != dp) {
			bad_flatpixel_counter++;
			// leave pixels as they are, they will be fixed in
			// the interplation step
			//ip = 0;
		} else {
			// recalibrate the pixel
			FlatPixelType	v = ip / dp;
			if (v > max) {
				ip = max;
			} else {
				ip = v;
			}
		}
		image.pixels[offset] = ip;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "bad pixels: %d in image, %d in flat",
		bad_imagepixel_counter, bad_flatpixel_counter);

	// don't interpolate if the interpolation_distance is 0
	if (0 == interpolation_distance) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "don't interpolate bad pixels");
		return;
	}

	// interpolate all the pixels 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "interpolationg NaNs in flat");
	int	interpolation_counter = 0;
	for (int x = 0; x < image.size().width(); x++) {
		for (int y = 0; y < image.size().height(); y++) {
			FlatPixelType	dp = flat.pixel(x, y);
			if (dp == dp) {
				interpolation_counter++;
				continue;
			}
			debug(LOG_DEBUG, DEBUG_LOG, 0, "found NaN at (%d,%d)",
				x, y);
			int	counter = 0;
			double	sum = 0;
			// try the neighbors
			int	X = x + interpolation_distance;
			if (image.size().contains(X, y)) {
				ImagePixelType	p = image.pixel(X, y);
				sum += p;
				counter++;
			}
			X = x - interpolation_distance;
			if (image.size().contains(X, y)) {
				ImagePixelType	p = image.pixel(X, y);
				sum += p;
				counter++;
			}
			int	Y = y + interpolation_distance;
			if (image.size().contains(x, Y)) {
				ImagePixelType	p = image.pixel(x, Y);
				sum += p;
				counter++;
			}
			Y = y - interpolation_distance;
			if (image.size().contains(x, Y)) {
				ImagePixelType	p = image.pixel(x, Y);
				sum += p;
				counter++;
			}
			if (counter > 0) {
				sum = (1. / counter) * sum;
			} else {
				// as a fall back, use the original pixel
				sum = image.pixel(x, y);
			}
			image.pixel(x, y) = sum;
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"interpolated value at (%d,%d) = %f (from %d)",
				x, y, sum, counter);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d interpolated pixels",
		interpolation_counter);
}

#define	flat_correct_for(T)						\
{									\
	Image<T>	*timage	= dynamic_cast<Image<T> *>(&*image);	\
	if (NULL != timage) {						\
		flat_correct(*timage, flat, interpolation_distance);	\
		return;							\
	}								\
}

template<typename FlatPixelType>
void	flat_correct_typed(ImagePtr image,
		const Image<FlatPixelType>& flat,
		const int interpolation_distance = 0) {
	flat_correct_for(unsigned char);
	flat_correct_for(unsigned short);
	flat_correct_for(unsigned int);
	flat_correct_for(unsigned long);
	flat_correct_for(double);
	flat_correct_for(float);
	std::string	msg("flat correction only for primitive types");
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

//////////////////////////////////////////////////////////////////////
// FlatCorrector implementation
//////////////////////////////////////////////////////////////////////
FlatCorrector::FlatCorrector(const ImagePtr _flat,
	const ImageRectangle _rectangle)
	: Corrector(_flat, _rectangle) {
}

/**
 * \brief perform flat correction
 *
 * Subtract the flat image from the argument image. This is done in place,
 * as most quite, the uncorrected image is no longer needed. If a new image
 * is required, first create the new image, then apply the flat corrector in
 * place.
 *
 * \param image		image to flat correct
 * \param interpolation_distance	
 *			how far away the pixels are between which to interpolate
 *			pixels that are NaN in the flat
 *			When flat correcting Bayer-images, it should be set 
 *			to 2, otherwise 1. If 0, no interpolation is performed
 */
void	FlatCorrector::operator()(ImagePtr image, const int interpolation_distance) const {
	Image<float>	*fp = dynamic_cast<Image<float> *>(&*calibrationimage);
	Image<double>	*dp = dynamic_cast<Image<double> *>(&*calibrationimage);
	if (NULL != fp) {
		flat_correct_typed<float>(image, *fp, interpolation_distance);
		return;
	}
	if (NULL != dp) {
		flat_correct_typed<double>(image, *dp, interpolation_distance);
		return;
	}
	std::string	msg("flat image must be of floating point type");
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

} // calibration
} // astro
