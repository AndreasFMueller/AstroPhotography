/*
 * CGFilter.cpp -- get the center of gravity
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFilter.h>
#include <limits>
#include <AstroDebug.h>

using namespace astro::adapter;

namespace astro {
namespace image {
namespace filter {

static long imagecounter = 0;

/**
 * \brief Compute the center of gravity of all pixels
 */
Point	CGFilter::operator()(const ConstImageAdapter<double>& image) {
	BorderFeatherAdapter<double>	feather(image, _radius);
	
	Point	sum;
	double	totalweight = 0;
	int	w = image.getSize().width();
	int	h = image.getSize().height();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			double	v = feather.pixel(x, y);
			if (v != v) {
				continue;
			}
			if (v >= std::numeric_limits<double>::infinity()) {
				continue;
			}
			if (v <= -std::numeric_limits<double>::infinity()) {
				continue;
			}
			sum = sum + Point(x, y) * v;
			totalweight += v;
		}
	}
	return sum * (1. / totalweight);
}

} // namespace filter
} // namespace image
} // namespace astro
