/*
 * Output.cpp -- output of basic types
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Output.h>

namespace Astro {

std::ostream&	operator<<(std::ostream& out, const Astro::BinningMode& mode) {
	out << mode.x << "x" << mode.y;
	return out;
}

std::ostream&	operator<<(std::ostream& out, const Astro::ImagePoint& point) {
	out << "(" << point.x << "," << point.y << ")";
	return out;
}

std::ostream&	operator<<(std::ostream& out, const Astro::ImageSize& size) {
	out << size.width << "x" << size.height;
	return out;
}

std::ostream&   operator<<(std::ostream& out,
			const Astro::ImageRectangle& rectangle) {
	out << rectangle.size << "@" << rectangle.origin;
	return out;
}

} // namespace astro
