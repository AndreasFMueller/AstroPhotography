/*
 * AiryImage.cpp
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConvolve.h>
#include <AstroDebug.h>
#include <cmath>

namespace astro {
namespace image {

/**
 * \brief Construct airy image adapter
 *
 * \param size			image size
 * \param a			radius of the aperture
 * \param angularpixelsize	size of a pixel in radians per pixel
 * \param lambda		wave length of the leight, default is
 *				visible light
 */
AiryImage::AiryImage(const ImageSize& size, const ImagePoint& center, double a,
	double angularpixelsize, double lambda)
	: ConstImageAdapter<double>(size), _center(center), _a(a),
	   _angularpixelsize(angularpixelsize) {
	_k = 2 * M_PI / lambda;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "airy: a = %f, k = %g, ang = %g",
		_a, _k, _angularpixelsize);
}

static inline double	sqr(double x) { return x * x; }

/**
 * \brief Compute the value of a airy disk pixel
 */
double	AiryImage::pixel(int x, int y) const {
	double	r = hypot(x - _center.x(), y - _center.y());
	if (r == 0) {
		r = 0.1;
	}
	r *= _angularpixelsize;
	double	X = _k * _a * r;
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "%d,%d -> %f", x, y, X);
	return sqr(j1(X) / X);
}

} // namespace image
} // namespace astro
