/*
 * ImagerRectangle.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <ImagerRectangle.h>
#include <sstream>

namespace snowgui {

astro::RaDec	ImagerRectangle::point(float x, float y) const {
	astro::RaDec	result;
	astro::TwoAngles	z(x * _size.a1(), y * _size.a2());
	result.ra()  =  z.a1() * cos(_azimuth) + z.a2() * sin(_azimuth);
	result.dec() = -z.a1() * sin(_azimuth) + z.a2() * cos(_azimuth);
	return result;
}

std::string	ImagerRectangle::toString() const {
	std::ostringstream 	out;
	out << _size.toString() << " azimuth=" << _azimuth.degrees();
	return out.str();
}

} // namespace snowgui
