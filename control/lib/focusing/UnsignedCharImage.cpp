/*
 * UnsignedCharImage.cpp -- convert an image to unsigned char
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include <AstroAdapter.h>
#include <AstroFilter.h>

using namespace astro::image::filter;

namespace astro {
namespace focusing {

/**
 * \brief Convert the image to unsigned char pixel type
 */

#define convert_to_unsigned_char_scaled(image, Pixel, topvalue)	\
if (NULL == result) {							\
	Image<Pixel>	*imagep = dynamic_cast<Image<Pixel> *>(&*image);\
	if (NULL != imagep) {						\
		double	maxvalue = Max<Pixel, double>().filter(*imagep);\
		result = new Image<unsigned char>(*imagep,		\
			(double)topvalue / maxvalue);			\
	}								\
}

#define	convert_to_unsigned_char(image, Pixel)				\
	convert_to_unsigned_char_scaled(image, Pixel, std::numeric_limits<Pixel>::max())

/**
 * \brief Extract and rescale the image as the green channel
 *
 * Independently of the pixel type of the focus camera, convert the image
 * to 8 bit and rescale the values so that they use the full range of the
 * camera.
 */
Image<unsigned char>	*UnsignedCharImage(ImagePtr image) {
	Image<unsigned char>	*result = NULL;
	convert_to_unsigned_char(image, unsigned char);
	convert_to_unsigned_char(image, unsigned short);
	convert_to_unsigned_char(image, unsigned int);
	convert_to_unsigned_char(image, unsigned long);
	convert_to_unsigned_char_scaled(image, float, 1);
	convert_to_unsigned_char_scaled(image, double, 1);
	if (NULL == result) {
		throw std::runtime_error("cannot convert image to 8bit");
	}

	// return the converted result
	return result;
}

} // namespace focusing
} // namespace astro
