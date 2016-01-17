/*
 * BasicSummary.cpp -- statistical information about a guiding run
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <time.h>

namespace astro {
namespace guiding {

BasicSummary::BasicSummary(double alpha) : _alpha(alpha) {
	time(&starttime);
}

Point	BasicSummary::averageoffset() const {
	return _average;
}

inline static double	sqr(const double& x) {
	return x * x;
}

Point	BasicSummary::variance() const {
	double	vx = average2.x() - sqr(_average.x());
	double	vy = average2.y() - sqr(_average.y());
	return Point(vx, vy);
}

void	BasicSummary::variance(const Point& v) {
	average2.setX(sqr(v.x()) + sqr(_average.x()));
	average2.setY(sqr(v.y()) + sqr(_average.y()));
}

void	BasicSummary::addPoint(const Point& offset) {
	lastoffset = offset;
	double	x = (1 - _alpha) * _average.x() + _alpha * offset.x();
	double	y = (1 - _alpha) * _average.y() + _alpha * offset.y();
	_average.setX(x);
	_average.setY(y);
	x = (1 - _alpha) * average2.x() + _alpha * sqr(offset.x());
	y = (1 - _alpha) * average2.y() + _alpha * sqr(offset.y());
	average2.setX(x);
	average2.setY(y);
}

} // namespace guiding
} // namespace astro
