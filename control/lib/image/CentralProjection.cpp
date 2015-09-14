/*
 * CentralProjection.cpp -- implementation of central projection
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
CentralProjection::CentralProjection(const RaDec& _center)
	: SphereProjection(_center) {
}

/**
 * \brief Project a unit sphere onto a plane
 */
Point	CentralProjection::operator()(const RaDec& x) const {
	UnitVector	X(x);
	double	lambda = 1. / (X * center());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "lambda = %f", lambda);
	Vector	v =  X * lambda;
	return Point(v * right(), v * up());
}

} // namespace transform
} // namespace image
} // namespace astro
