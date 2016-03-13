/*
 * EuclideanDisplacementConvolve.cpp -- implementation of the convolution
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGroup.h>
#include <AstroDebug.h>

namespace astro {
namespace image {
namespace transform {

static void	add(Image<double>& result,
			const ConstImageAdapter<double>& image, double f) {
	int	w = result.getSize().width();
	int	h = result.getSize().height();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			result.pixel(x, y) += image.pixel(x, y) * f;
		}
	}
}

template<>
Image<double>	*EuclideanDisplacementConvolve<double>::operator()(
			const ConstImageAdapter<double>& image) const {
	Image<double>	*result = new Image<double>(image.getSize());
	int	w = image.getSize().width();
	int	h = image.getSize().height();
	double	radius = hypot(w, h);

	int	xmin = floor(-radius - w);
	int	xmax = -xmin;
	int	ymin = floor(-radius - h);
	int	ymax = -ymin;
	int	anglemin = 0;
	int	anglemax = _angleresolution;
	double	delta = 2 * M_PI / _angleresolution;

	for (int angle = anglemin; angle < anglemax; angle++) {
		double	a = angle * delta;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "angle(%d) = %f", angle, a);
		for (int x = xmin; x <= xmax; x++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "x = %d", x);
			for (int y = ymin; y <= ymax; y++) {
				//debug(LOG_DEBUG, DEBUG_LOG, 0, "y = %d", y);
				Point	translation(x, y);
				EuclideanDisplacement	disp(a, translation);
				double	v = _f(disp);
				if (v != 0) {
					EuclideanDisplacementAdapter<double>	da(image, disp);
					add(*result, da, v);
				}
			}
		}
	}

	return result;
}

} // namespace transform
} // namespace image
} // namespace astro
