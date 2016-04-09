/*
 * Rectangle.cpp -- rectangle implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTypes.h>
#include <AstroFormat.h>

namespace astro {

Rectangle::Rectangle(const std::set<Point>& points)
	: Point(Point::lowerleft(points)), Size(points) {
}

std::string	Rectangle::toString() const {
	return stringprintf("%s@%s", Size::toString().c_str(),
		Point::toString().c_str());
}

std::ostream&	operator<<(std::ostream& out, const Rectangle& rectangle) {
	out << rectangle.toString();
	return out;
}

bool    Rectangle::contains(const Point& point) const {
        return ((origin().x() <= point.x())
			&& (point.x() <= origin().x() + size().width() - 1) &&
		(origin().y() <= point.y())
			&& (point.y() <= origin().y() + size().height() - 1));
}

} // namespace astro
