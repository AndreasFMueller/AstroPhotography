/*
 * TrackingPoint.cpp - implementation of the TrackingPoint
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <AstroFormat.h>

namespace astro {
namespace guiding {

std::string	TrackingPoint::toString() const {
	return stringprintf("%.0f %s offset=%s correction=%s",
		type2string(type).c_str(),
		trackingoffset.toString().c_str(),
		correction.toString().c_str());
}

} // namespace guiding
} // namespace astro
