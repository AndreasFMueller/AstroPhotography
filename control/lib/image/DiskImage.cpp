/*
 * DiskImage.cpp -- image with a circular disk
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
 * \brief Construct Disk image adapter
 *
 * \param size			image size
 * \param center		center of the image
 * \param r			radial angle of the disk
 * \param angularpixelsize	size of a pixel in radians per pixel
 */
DiskImage::DiskImage(const ImageSize& size, const ImagePoint& center,
	double r, double angularpixelsize, double weight)
	: CircularImage(size, center, angularpixelsize, weight),
	  _r(r) {
	_interiorvalue = CircularImage::weight() / totalweight();
}


/**
 * \brief Compute the value of a airy disk pixel
 */
double	DiskImage::pixel(int x, int y) const {
	double	rr = r(x, y) * angularpixelsize();
	if (rr > _r) {
		return 0;
	}
	return _interiorvalue;
}

double	DiskImage::totalweight() const {
	return M_PI * sqr(_r);
}

} // namespace image
} // namespace astro
