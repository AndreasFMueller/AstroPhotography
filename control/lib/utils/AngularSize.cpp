/*
 * AngularSize.cpp
 *
 * (c) 2019 Prof Dr Andreas Müller Hochschule Rapperswil
 */
#include <AstroCoordinates.h>

namespace astro {

AngularSize::AngularSize(double pixelsize, double focallength)
	: Angle(atan(pixelsize / focallength)) {
	if (focallength <= 0) {
		std::string	msg = stringprintf("focallength must be "
			"positive: %f", focallength);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}

AngularSize::AngularSize(const Angle& angle) : Angle(angle) {
}

double	AngularSize::scaledPixel(double distance) const {
	return distance * tan(*this);
}

double	operator*(double r, const AngularSize& s) {
	return r * tan(s);
}

double	operator/(double r, const AngularSize& s) {
	return r / tan(s);
}

double	operator/(const Angle& angle, const AngularSize& s) {
	return angle.radians() / tan(s);
}

} // namespace astro
