/*
 * SterographicProjection.cpp -- implementation of stereographic projection
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProjection.h>
#include <AstroDebug.h>

namespace astro {
namespace image {
namespace transform {

/**
 * \brief Construct a stereographic projection object
 */
StereographicProjection::StereographicProjection(const RaDec& _center)
	: center(_center) {
	UnitVector	north(RaDec::north_pole);
	right = center.cross(north);
	up = right.cross(center);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "center: %s", center.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "right: %s", right.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "up: %s", up.toString().c_str());
}

/**
 * \brief Project a unit sphere onto a plane
 */
Point	StereographicProjection::operator()(const RaDec& x) const {
	UnitVector	X(x);
	double	lambda = 1. / (1 + X * center);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "lambda = %f", lambda);
	Vector	v =  X * lambda + center * (lambda - 1);
	return Point(v * right, v * up);
}

} // namespace project
} // namespace image
} // namespace astro
