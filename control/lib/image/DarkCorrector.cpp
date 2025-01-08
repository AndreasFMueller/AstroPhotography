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

/**
 * \brief Type dark correctors
 *
 * Dark correction can be applied to any type of image, with varying
 * primitive pixel types. These templates perform dark correction
 * based on the various possible pixel types.
 *
 * \param interpolation_distance	how wide the grid is for interpolation
 *					use 2 for bayer sensors, 1 for mono
 *					sensors and 0 for no interpolation
 */
template<typename ImagePixelType, typename DarkPixelType>
static void	dark_correct(Image<ImagePixelType>& image,
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
void	dark_correct_typed(ImagePtr image,
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

/**
 * \brief Constructor for DarkCorrector
 * * \param _dark		the dark calibration image
 * \param _rectangle	the rectangle of interest
 */
DarkCorrector::DarkCorrector(const ImagePtr _dark,
	const ImageRectangle _rectangle)
	: Corrector(_dark, _rectangle) {
}

#define dark_correct_typed0(T)						\
{									\
	Image<T>	*ip = dynamic_cast<Image<T> *>(&*calibrationimage);\
	if (NULL != ip) {						\
		WindowAdapter<T>	wa(*ip, rectangle);		\
		dark_correct_typed<T>(image, wa);			\
	}								\
}

/**
 * \brief perform dark correction
 *
 * Subtract the dark image from the argument image. This is done in place,
 * as most of the time, the uncorrected image is no longer needed.
 * If a new image is required, first create the new image, then apply
 * the dark corrector in place.
 *
 * \param image     image to dark correct
 * \param interpolation_distance
 */
void	DarkCorrector::operator()(ImagePtr image,
		const int interpolation_distance) const {
	// try the floating point pixel types for calibration
	dark_correct_typed0(float)
	dark_correct_typed0(double)

	// there are no other possible pixel types for the calibration image
	// so we can just go on the perform the interpolation, which is done
	// using the base class operator()
	Corrector::operator()(image, interpolation_distance);
}

} // calibration
} // astro
