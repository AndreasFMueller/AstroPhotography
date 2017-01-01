/*
 * GaussImage.cpp -- image with a gaussian intensity profile
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConvolve.h>
#include <AstroDebug.h>
#include <cmath>

namespace astro {
namespace image {

/**
 * \brief Construct Gauss image adapter
 *
 * \param size			image size
 * \param center		center of the image
 * \param sigma			standard deviation in radians
 * \param angularpixelsize	size of a pixel in radians per pixel
 */
GaussImage::GaussImage(const ImageSize& size, const ImagePoint& center,
	double sigma, double angularpixelsize, double totalweight)
	: CircularImage(size, center, angularpixelsize, totalweight),
	  _sigma(sigma) {
	_n = 1 / (2 * M_PI * (_sigma / CircularImage::angularpixelsize()));
}

static inline double	sqr(double x) { return x * x; }

/**
 * \brief Compute the value of a Gauss distribution pixel
 */
double	GaussImage::pixel(int x, int y) const {
	double	rr = r(x, y) / _sigma;
	return weight() * exp(-sqr(rr)) * _n;
}

TiledGaussImage::TiledGaussImage(const ImageSize& size,
	double sigma, double angularpixelsize, double totalweight)
	: GaussImage(size, ImagePoint(), sigma, angularpixelsize, totalweight),
	  _sigma(sigma) {
	_n = 1 / (2 * M_PI * (_sigma / CircularImage::angularpixelsize()));
	_w = size.width();
	_h = size.height();
}

double	TiledGaussImage::pixel(int x, int y) const {
	if (x >= _w / 2) { x = _w - x; }
	if (y >= _h / 2) { y = _h - y; }
	return GaussImage::pixel(x, y);
}

} // namespace image
} // namespace astro
