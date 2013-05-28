/**
 * ImageProperties.cpp -- find out properties of an image
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <limits>

namespace astro {
namespace image {

bool	isColorImage(const ImagePtr& image) {
	if (dynamic_cast<Image<unsigned char> *>(&*image)) {
		return false;
	}
	if (dynamic_cast<Image<unsigned short> *>(&*image)) {
		return false;
	}
	if (dynamic_cast<Image<unsigned long> *>(&*image)) {
		return false;
	}
	if (dynamic_cast<Image<float> *>(&*image)) {
		return false;
	}
	if (dynamic_cast<Image<double> *>(&*image)) {
		return false;
	}
	return false;
}

bool	isMonochromeImage(const ImagePtr& image) {
	return !isColorImage(image);
}

} // namespace image
} // namespace astro
