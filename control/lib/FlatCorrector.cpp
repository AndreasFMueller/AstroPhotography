/**
 * FlatCorrector.cpp -- perform flat correction
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCalibration.h>
#include <AstroFilter.h>
#include <PixelValue.h>
#include <limits>
#include <debug.h>
#include <stdexcept>
#include <vector>
#include <Format.h>

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
		const Image<FlatPixelType>& flat) {
	ImagePixelType	max = std::numeric_limits<ImagePixelType>::max();

	// first check that image sizes match
	if (image.size != flat.size) {
		std::string	msg = stringprintf("size: image %s != flat %s",
			image.size.toString().c_str(),
			flat.size.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// correct all pixels
	for (size_t offset = 0; offset < image.size.pixels; offset++) {
		ImagePixelType	ip = image.pixels[offset];
		// skip NaN pixels
		if (ip != ip) {
			continue;
		}
		FlatPixelType	dp = flat.pixels[offset];
		// turn off (make nan) pixels that are marked nan in the flat
		if (dp != dp) {
			ip = 0;
		} else {
			FlatPixelType	v = ip / dp;
			if (v > max) {
				ip = max;
			} else {
				ip = v;
			}
		}
		image.pixels[offset] = ip;
	}
}

#define	flat_correct_for(T)						\
{									\
	Image<T>	*timage	= dynamic_cast<Image<T> *>(&*image);	\
	if (NULL != timage) {						\
		flat_correct(*timage, flat);				\
		return;							\
	}								\
}

template<typename FlatPixelType>
void	flat_correct_typed(ImagePtr& image,
		const Image<FlatPixelType>& flat) {
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
FlatCorrector::FlatCorrector(const ImagePtr& _flat) : flat(_flat) {
	// We want flat images to be of float or double type
	Image<float>	*fp = dynamic_cast<Image<float> *>(&*flat);
	Image<double>	*dp = dynamic_cast<Image<double> *>(&*flat);
	if ((NULL != fp) || (NULL != dp)) {
		return;
	}
	std::string	msg("flat image must be of floating point type");
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief perform flat correction
 *
 * Subtract the flat image from the argument image. This is done in place,
 * as most quite, the uncorrected image is no longer needed. If a new image
 * is required, first create the new image, then apply the flat corrector in
 * place.
 * \param image     image to flat correct
 */
void	FlatCorrector::operator()(ImagePtr& image) const {
	Image<float>	*fp = dynamic_cast<Image<float> *>(&*flat);
	Image<double>	*dp = dynamic_cast<Image<double> *>(&*flat);
	if (NULL != fp) {
		flat_correct_typed<float>(image, *fp);
		return;
	}
	if (NULL != dp) {
		flat_correct_typed<double>(image, *dp);
		return;
	}
	std::string	msg("flat image must be of floating point type");
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

} // calibration
} // astro
