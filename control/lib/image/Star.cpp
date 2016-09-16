/*
 * Star.cpp -- star abstraction for star extraction
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTransform.h>

namespace astro {
namespace image {
namespace transform {

bool	Star::operator<(const Star& other) const {
	return _brightness < other._brightness;
}

std::string	Star::toString() const {
	return stringprintf("%s brightness=%.1f", Point::toString().c_str(),
		_brightness);
}

Star::operator	std::string() const {
	return toString();
}

} // namespace transform
} // namespace image
} // namespace astro
