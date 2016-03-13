/*
 * EuclideanDisplacement.cpp -- implementation of displacement group
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGroup.h>

namespace astro {
namespace image {
namespace transform {

void	EuclideanDisplacement::setup() {
	c = cos(_a);
	s = sin(_a);
}

Point	EuclideanDisplacement::rotate(const Point& other) const {
	return Point(c * other.x() + s * other.y(),
		-s * other.x() + c * other.y());
}

EuclideanDisplacement::EuclideanDisplacement() : _a(0) {
	setup();
}

EuclideanDisplacement::EuclideanDisplacement(double angle) : _a(angle) {
	setup();
}

EuclideanDisplacement::EuclideanDisplacement(Point translation)
	: _a(0), _t(translation) {
	setup();
}

EuclideanDisplacement::EuclideanDisplacement(double angle, Point translation)
	: _a(angle), _t(translation) {
	setup();
}

EuclideanDisplacement	EuclideanDisplacement::inverse() const {
	EuclideanDisplacement	result(-angle(), -translation());
	return result;
}

EuclideanDisplacement	EuclideanDisplacement::operator*(
				const EuclideanDisplacement& other) const {
	EuclideanDisplacement	result(angle() + other.angle(),
		rotate(other.translation()) + translation());
	return result;
}

EuclideanDisplacement	EuclideanDisplacement::operator/(
				const EuclideanDisplacement& other) const {
	return *this * other.inverse();
}

Point	EuclideanDisplacement::operator()(const ImagePoint& p) const {
	return rotate(p) + translation();
}

Point	EuclideanDisplacement::operator()(const Point& p) const {
	return rotate(p) + translation();
}

} // namespace transform
} // namespace image
} // namespace astro
