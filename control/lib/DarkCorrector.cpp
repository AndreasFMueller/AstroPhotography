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
		const Image<DarkPixelType>& dark) {
	// first check that image sizes match
	if (image.size != dark.size) {
		std::string	msg = stringprintf("size: image %s != dark %s",
			image.size.toString().c_str(),
			dark.size.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// correct all pixels
	for (size_t offset = 0; offset < image.size.getPixels(); offset++) {
		ImagePixelType	ip = image.pixels[offset];
		// skip NaN pixels
		if (ip != ip) {
			continue;
		}
		DarkPixelType	dp = dark.pixels[offset];
		// turn off (make nan) pixels that are marked nan in the dark
		if (dp != dp) {
			ip = 0;
		} else {
			if (ip > dp) {
				ip = ip - dp;
			} else {
				ip = 0;
			}
		}
		image.pixels[offset] = ip;
	}
}

#define	dark_correct_for(T)						\
{									\
	Image<T>	*timage	= dynamic_cast<Image<T> *>(&*image);	\
	if (NULL != timage) {						\
		dark_correct(*timage, dark);				\
		return;							\
	}								\
}

template<typename DarkPixelType>
void	dark_correct_typed(ImagePtr& image,
		const Image<DarkPixelType>& dark) {
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
DarkCorrector::DarkCorrector(const ImagePtr& _dark) : dark(_dark) {
	// We want dark images to be of float or double type
	Image<float>	*fp = dynamic_cast<Image<float> *>(&*dark);
	Image<double>	*dp = dynamic_cast<Image<double> *>(&*dark);
	if ((NULL != fp) || (NULL != dp)) {
		return;
	}
	std::string	msg("dark image must be of floating point type");
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief perform dark correction
 *
 * Subtract the dark image from the argument image. This is done in place,
 * as most quite, the uncorrected image is no longer needed. If a new image
 * is required, first create the new image, then apply the dark corrector in
 * place.
 * \param image     image to dark correct
 */
void	DarkCorrector::operator()(ImagePtr& image) const {
	Image<float>	*fp = dynamic_cast<Image<float> *>(&*dark);
	Image<double>	*dp = dynamic_cast<Image<double> *>(&*dark);
	if (NULL != fp) {
		dark_correct_typed<float>(image, *fp);
		return;
	}
	if (NULL != dp) {
		dark_correct_typed<double>(image, *dp);
		return;
	}
	std::string	msg("dark image must be of floating point type");
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

} // calibration
} // astro
