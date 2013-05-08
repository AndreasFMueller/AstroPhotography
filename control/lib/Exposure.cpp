/*
 * Exposure.cpp -- implementation of the exposure class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <Format.h>
#include <math.h>

namespace astro {
namespace camera {

Exposure::Exposure() : exposuretime(1.), gain(1.), limit(INFINITY) {
}

Exposure::Exposure(const astro::image::ImageRectangle& _frame,
	float _exposuretime)
                : frame(_frame), exposuretime(_exposuretime), gain(1.),
		  limit(INFINITY) {
}

std::string	Exposure::toString() const {
	return stringprintf("%dx%d@(%d,%d)/%s for %.3fms",
		frame.size.width, frame.size.height,
		frame.origin.x, frame.origin.y,
		mode.toString().c_str(), exposuretime);
}

std::ostream&	operator<<(std::ostream& out, const Exposure& exposure) {
	return out << exposure.toString();
}

} // namespace camera
} // namespace astro
