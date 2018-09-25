/*
 * SphericalCoordinates.cpp -- spherical coordinate system implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCoordinates.h>
#include <cmath>
#include <stdexcept>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <regex>
#include <mutex>
#include <typeinfo>

namespace astro {

SphericalCoordinates::SphericalCoordinates(const LongLat& longlat)
	: TwoAngles(longlat.longitude(), Angle(M_PI / 2) - longlat.latitude()) {
}

SphericalCoordinates::SphericalCoordinates(const RaDec& radec)
	: TwoAngles(radec.ra(), Angle(M_PI / 2) - radec.dec()) {
}

Angle	operator-(const SphericalCoordinates& s1, const SphericalCoordinates& s2) {
	return UnitVector(s1).angle(UnitVector(s2));
}

} // namespace astro
