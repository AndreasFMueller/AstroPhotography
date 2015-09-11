/*
 * PointSpreadFunctionAdapter.cpp --  adapter for point spread functions
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroChart.h>

namespace astro {
namespace catalog {

PointSpreadFunctionAdapter::PointSpreadFunctionAdapter(const ImageSize& size,
	const ImagePoint& center, double angularpixelsize,
	const PointSpreadFunction& pointspreadfunction)
	: CircularImage(size, center, angularpixelsize, 1),
	  _pointspreadfunction(pointspreadfunction) {
}

double	PointSpreadFunctionAdapter::pixel(int x, int y) const {
	return _pointspreadfunction(r(x, y));
}

} // namespace catalog
} // namespace astro
