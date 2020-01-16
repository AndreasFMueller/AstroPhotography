/*
 * HotPixelBase.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller Hochschule Rapperswil
 */
#include <AstroAdapter.h>
#include <cmath>

namespace astro {
namespace adapter {

/**
 * \brief Compute mean and stddev and decide whether a pixel is hot
 *
 * \param x	x coordinate of the pixel to inspect
 * \param y	y coordinate of the pixel to instect
 */
HotPixelInfo	HotPixelBase::meanstddev(int x, int y) const {
	double	l = this->luminance(x, y);
	int	w = _size.width();
	int	h = _size.height();
	int	xmin = x - _search_radius;
	int	xmax = x + _search_radius;
	int	ymin = y - _search_radius;
	int	ymax = y + _search_radius;
	if (xmin < 0) { xmin = 0; }
	if (xmax >= w) { xmax = w - 1; }
	if (ymin < 0) { ymin = 0; }
	if (ymax >= h) { ymax = h - 1; }
	double	sum = 0;
	double	sum2 = 0;
	int	counter = 0;
	for (int xx = xmin; xx <= xmax; xx++) {
		for (int yy = ymin; yy <= ymax; yy++) {
			double	v = this->luminance(xx, yy);
			sum += v;
			sum2 += v * v;
			counter++;
		}
	}
	double	E = sum / counter;
	double	E2 = sum2 / counter;
	double	var = counter * (E2 - E * E) / (counter - 1);
	HotPixelInfo	result;
	result.mean = E;
	result.stddev = sqrt(var);
	//debug(LOG_DEBUG, DEBUG_LOG, 0,
	//	"counter = %d, mean = %f, stddev = %f, pixel = %f, multi = %d",
	//	counter, result.mean, result.stddev, l,
	//	_stddev_multiplier);
	result.is_hot = abs(result.mean - l) > _stddev_multiplier * result.stddev;
	if (result.is_hot) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"found hot pixel at (%d,%d) = %f, mean=%f,stddev=%f",
			x, y, l, result.mean, result.stddev);
		const_cast<std::list<ImagePoint>&>(_bad_pixels).push_back(ImagePoint(x,y));
	}
	return result;
}

} // namespace adapter
} // namespace astro
