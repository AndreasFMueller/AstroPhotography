/**
 * FlatCorrector.cpp -- perform flat correction
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCalibration.h>
#include <AstroFilter.h>
#include <AstroAdapter.h>
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

/**
 * \brief Type flat correctors
 *
 * Flat correction can be applied to any type of image, with varying
 * primitive pixel types. These templates perform flat correction
 * based on the various possible pixel types
 *
 * \param image		the image to correct
 * \param flat		the flat image to use for the correction
 */
template<typename ImagePixelType, typename FlatPixelType>
static void	flat_correct(Image<ImagePixelType>& image,
		const Image<FlatPixelType>& flat) {
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
}

#define	flat_correct_for(T)						\
{									\
	Image<T>	*timage	= dynamic_cast<Image<T> *>(&*image);	\
	if (NULL != timage) {						\
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found %s pixel type",	\
			#T);						\
		flat_correct(*timage, flat);				\
		return;							\
	}								\
}

template<typename FlatPixelType>
static void	flat_correct_typed(ImagePtr image,
			const Image<FlatPixelType>& flat) {
	flat_correct_for(unsigned char)
	flat_correct_for(unsigned short)
	flat_correct_for(unsigned int)
	flat_correct_for(unsigned long)
	flat_correct_for(double)
	flat_correct_for(float)
	std::string	msg("flat correction only for primitive types");
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Constructor for the FlatCorrector
 *
 * \param _flat		the flat image to use
 * \param _rectangle	the rectangle inside the flat to concentrate on
 */
FlatCorrector::FlatCorrector(const ImagePtr _flat,
	const ImageRectangle _rectangle)
	: Corrector(_flat, _rectangle) {
}

#define flat_correct_typed0(T)						\
{									\
	Image<T>	*ip = dynamic_cast<Image<T> *>(&*calibrationimage);\
	if (NULL != ip) {						\
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found %s cal image type",\
			#T);						\
		adapter::WindowAdapter<T>    window(*ip, rectangle);	\
		flat_correct_typed<T>(image, window);			\
	}								\
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
void	FlatCorrector::operator()(ImagePtr image,
		const int interpolation_distance) const {
	// try float calibration image type
	flat_correct_typed0(float)
	flat_correct_typed0(double)

	// interpolate the image (this is controlled by the value
	// of the interpolation_distance)
	Corrector::operator()(image, interpolation_distance);
}

} // calibration
} // astro
