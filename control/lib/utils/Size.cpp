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

Size::Size(const std::string& sizestring) {
	// split the string at the x
	auto i = sizestring.find("x");
	std::string	first = sizestring.substr(0, i);
	_width = stod(first);
	std::string	second = sizestring.substr(i + 1);
	_height = stod(second);
}

std::string	Size::toString() const {
	return stringprintf("%fx%f", _width, _height);
}

std::ostream&	operator<<(std::ostream& out, const Size& size) {
	out << size.toString();
	return out;
}

bool	Size::contains(const Point& point) const {
	return (0 <= point.x()) && (point.x() <= _width - 1) &&
		(0 <= point.y()) && (point.y() <= _height - 1);
}

double	Size::diagonal() const {
	return hypot(_width, _height);
}

} // namespace astro
