/*
 * Size.cpp -- Size of rectangles
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTypes.h>
#include <AstroFormat.h>

namespace astro {

void	Size::setup(const Point& lowerleft, const Point& upperright) {
	_width = upperright.x() - lowerleft.x() + 1;
	if (_width <= 0) {
		throw std::runtime_error("negative horizontal size");
	}
	_height = upperright.y() - lowerleft.y() + 1;
	if (_height <= 0) {
		throw std::runtime_error("negative vertical size");
	}
}
Size::Size(const Point& lowerleft, const Point& upperright) {
	setup(lowerleft, upperright);
}

Size::Size(const std::set<Point>& points) {
	setup(Point::lowerleft(points), Point::upperright(points));
}

std::string	Size::toString() const {
	return stringprintf("%fx%f", _width, _height);
}

std::ostream&	operator<<(std::ostream& out, const Size& size) {
	out << size.toString();
	return out;
}

} // namespace astro
