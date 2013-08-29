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
		const ConstImageAdapter<DarkPixelType>& dark) {
	// first check that image sizes match
	if (image.size() != dark.getSize()) {
		std::string	msg = stringprintf("size: image %s != dark %s",
			image.size().toString().c_str(),
			dark.getSize().toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// correct all pixels
	for (unsigned int x = 0; x < image.size().width(); x++) {
		for (unsigned int y = 0; y < image.size().height(); y++) {

			ImagePixelType	ip = image.pixel(x, y);
			// skip NaN pixels
			if (ip != ip) {
				continue;
			}
			DarkPixelType	dp = dark.pixel(x, y);
			// turn off (make nan) pixels that are marked nan
			// in the dark
			if (dp != dp) {
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
		const ConstImageAdapter<DarkPixelType>& dark) {
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
DarkCorrector::DarkCorrector(const ImagePtr& _dark,
	const ImageRectangle _rectangle)
	: Corrector(_dark, _rectangle) {
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
	Image<float>	*fp = dynamic_cast<Image<float> *>(&*calibrationimage);
	Image<double>	*dp = dynamic_cast<Image<double> *>(&*calibrationimage);
	if (NULL != fp) {
		WindowAdapter<float>	wa(*fp, rectangle);
		dark_correct_typed<float>(image, wa);
		return;
	}
	if (NULL != dp) {
		WindowAdapter<double>	wa(*dp, rectangle);
		dark_correct_typed<double>(image, wa);
		return;
	}
	std::string	msg("dark image must be of floating point type");
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

} // calibration
} // astro
