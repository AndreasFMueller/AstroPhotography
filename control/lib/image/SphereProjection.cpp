/*
 * SphereProjection.cpp -- 
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProjection.h>

namespace astro {
namespace image {
namespace transform {

SphereProjection::SphereProjection(const RaDec& center) : _center(center) {
	UnitVector	north(RaDec::north_pole);
	_right = _center.cross(north);
	_up = _right.cross(_center);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "center: %s",
		_center.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "right: %s",
		_right.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "up: %s",
		_up.toString().c_str());
}

} // namepsace transform
} // namespace image
} // namespace astro
