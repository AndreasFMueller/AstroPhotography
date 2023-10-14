/*
 * BackProjection.cpp -- back projection
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Radon.h>

namespace astro {
namespace image {
namespace radon {

/**
 * \brief Back projection constructor computes the back projection
 */
BackProjection::BackProjection(const ImageSize& size,
	const ConstImageAdapter<double>& radon)
	: ConstImageAdapter<double>(size), _radon(radon),
	  _backprojection(size) {

	for (int h = 0; h < _radon.getSize().height(); h++) {
		anglesum(h);
	}
}

/**
 * \brief Add terms for a given angle
 */
void	BackProjection::anglesum(int angleindex) {
	// determine the center
	ImageSize	size = _backprojection.getSize();
	ImagePoint	center = size.center();
	int	bwidth = size.width();
	int	bheight = size.height();

	// compute the angle step
	double	angle = angleindex * M_PI / _radon.getSize().height();
	double	c = cos(angle), s = sin(angle);

	// determine range of pixels in a row
	int	w = _radon.getSize().width();
	int	w2 = w / 2;
#pragma omp parallel for
	for (int x = 0; x < bwidth; x++) {
		for (int y = 0; y < bheight; y++) {
			int	r = w2 + c * (x - center.x())
					+ s * (y - center.y());
			if ((0 <= r) && (r < w)) {
				double	v = _radon.pixel(r, angleindex);
				_backprojection.pixel(x, y)
					= _backprojection.pixel(x, y) + v;
			}
		}
	}
}

/**
 * \brief Access to the image pixels
 */
double	BackProjection::pixel(int x, int y) const {
	return _backprojection.pixel(x, y);
}

} // namespace radon
} // namespace image
} // namespace astro
