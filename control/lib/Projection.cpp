/*
 * Projection.cpp -- implementation of Projection
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProjection.h>

using namespace astro::image::transform;

namespace astro {
namespace image {
namespace project {

//////////////////////////////////////////////////////////////////////
// Projection implementation
//////////////////////////////////////////////////////////////////////

float	Projection::w(float r) const {
	float	x = r * r;
	return 1. + x * (b[0] + x * b[1]);
}

Projection::Projection() {
	b[0] = 0.; b[1] = 0.;
}

Point	Projection::operator()(const Point& p) const {
	Point	result = Transform::operator()(p);
	return result * (double)w(result.abs());
}

} // namespace project
} // namespace image
} // namespace astro
