/*
 * AmplifierGlowImage.cpp -- 
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConvolve.h>

namespace astro {
namespace image {

AmplifierGlowImage::AmplifierGlowImage(const ImageSize& size,
	const ImagePoint& center, double angularpixelsize, double weight,
	double r)
	: CircularImage(size, center, angularpixelsize, weight), _r(r) {
}

double	AmplifierGlowImage::pixel(int x, int y) const {
	double	value = 0.5 * weight() * _r / r(x, y);
	if (value > weight()) {
		return weight();
	}
	return value;
}

} // namespace image
} // namespace astro
