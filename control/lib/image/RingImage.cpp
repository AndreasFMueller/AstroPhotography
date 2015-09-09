/*
 * RingImage.cpp -- image with an annular image
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConvolve.h>
#include <AstroDebug.h>
#include <cmath>

namespace astro {
namespace image {

static inline double	sqr(double x) { return x * x; }

/**
 * \brief Construct Ring image adapter
 *
 * \param size			image size
 * \param center		center of the image
 * \param r			radial angle of the disk
 * \param angularpixelsize	size of a pixel in radians per pixel
 */
RingImage::RingImage(const ImageSize& size, const ImagePoint& center,
	double r_inner, double r_outer, double angularpixelsize,
	double totalweight)
	: CircularImage(size, center, angularpixelsize, totalweight),
	  _r_inner(r_inner), _r_outer(r_outer) {
	if (r_inner >= r_outer) {
		throw std::range_error("inner radius may not exceed outer radius");
	}
	_interiorvalue = weight() / (M_PI * (sqr(r_outer) - sqr(r_inner)));
}


/**
 * \brief Compute the value of a airy disk pixel
 */
double	RingImage::pixel(int x, int y) const {
	double	rr = angularpixelsize() * r(x, y);
	if ((rr > _r_outer) || (rr < _r_inner)) {
		return 0;
	}
	return _interiorvalue;
}

} // namespace image
} // namespace astro
