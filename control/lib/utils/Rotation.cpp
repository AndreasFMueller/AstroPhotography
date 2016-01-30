/*
 * Rotation.cpp -- implementation of rotation operator
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTypes.h>

namespace astro {

Point	Rotation::operator()(const Point& p) const {
	double	c = cos(_alpha);
	double	s = sin(_alpha);
	return Point(c * p.x() - s * p.y(), s * p.x() + s * p.y());
}

} // namespace astro
