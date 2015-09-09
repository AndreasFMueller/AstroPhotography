/*
 * CircularImage.cpp -- base class implementation for rotationally symmetric
 *                      images
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil 
 */
#include <AstroConvolve.h>

namespace astro {
namespace image {

CircularImage::CircularImage(const ImageSize& size, const ImagePoint& center,
	double angularpixelsize, double weight)
	: ConstImageAdapter<double>(size), _center(center),
	  _angularpixelsize(angularpixelsize) {
}

double	CircularImage::r(int x, int y) const {
	return hypot(x - _center.x(), y - _center.y());
}

} // namespace image
} // namespace astro
