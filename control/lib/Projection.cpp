/*
 * Projection.cpp -- implementation of Projection
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProjection.h>

namespace astro {
namespace image {
namespace transform {

//////////////////////////////////////////////////////////////////////
// Projection implementation
//////////////////////////////////////////////////////////////////////

Projection::Projection(double angle, const Point& translation,
	double scalefactor) 
	: Transform(angle, translation, scalefactor) {
}

double	Projection::w(double r) const {
	double	x = r * r;
	return 1. + x * (b[0] + x * b[1]);
}

Projection::Projection() {
	b[0] = 0.; b[1] = 0.;
}

Point	Projection::operator()(const Point& p) const {
	Point	result = Transform::operator()(p);
	return result * (double)w(result.abs());
}

double	Projection::operator[](int i) const {
	if (i < 6) {
		return Transform::operator[](i);
	}
	if (i < 8) {
		return b[i - 6];
	}
	throw std::range_error("out of range");
}

double&	Projection::operator[](int i) {
	if (i < 6) {
		return Transform::operator[](i);
	}
	if (i < 8) {
		return b[i - 6];
	}
	throw std::range_error("out of range");
}

} // namespace project
} // namespace image
} // namespace astro
