/**
 * ImageProperties.cpp -- find out properties of an image
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <limits>

namespace astro {
namespace image {

#define	iscolor(image, pixel)						\
	if (dynamic_cast<Image<pixel> *>(&*image)) {			\
		return false;						\
	}

bool	isColorImage(const ImagePtr& image) {
	iscolor(image, unsigned char)
	iscolor(image, unsigned short)
	iscolor(image, unsigned int)
	iscolor(image, unsigned long)
	iscolor(image, float)
	iscolor(image, double)
	return true;
}

bool	isMonochromeImage(const ImagePtr& image) {
	return !isColorImage(image);
}

} // namespace image
} // namespace astro
