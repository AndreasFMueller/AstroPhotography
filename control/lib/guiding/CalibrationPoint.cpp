/*
 * CalibrationPoint.cpp -- CalibrationPoint implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <AstroFormat.h>

namespace astro {
namespace guiding {

std::string	CalibrationPoint::toString() const {
	return stringprintf("t=%.1f %s -> %s", t, offset.toString().c_str(), 
		star.toString().c_str());
}

} // namespace guiding
} // namespace astro
