/*
 * Stars.cpp -- star implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Stars.h>

namespace astro {

inline double	sqr(const double x) {
	return x * x;
}

double	Star::intensity(const Point& where) const {
	return _peak * exp(-sqr(distance(where) / 3));
}

void	Star::magnitude(const double& magnitude) {
	_magnitude = magnitude;
	_peak = exp(-0.4 * _magnitude);
}

double	Nebula::intensity(const Point& where) const {
	return (distance(where) > _radius) ? 0 : _density;
}

void	StarField::createStar(const ImageSize& size, int overshoot) {
	int	x = (random() % (size.width() + 2 * overshoot)) - overshoot;
	int	y = (random() % (size.height() + 2 * overshoot)) - overshoot;
	double	magnitude = 1;
	
	addObject(StellarObjectPtr(new Star(Point(x, y), magnitude)));
}

StarField::StarField(const ImageSize& size, int overshoot,
	unsigned int nobjects) {
	for (unsigned int i = 0; i < nobjects; i++) {
		createStar(size, overshoot);
	}
}

void	StarField::addObject(StellarObjectPtr object) {
	objects.push_back(object);
}

double	StarField::intensity(const Point& where) const {
	std::vector<StellarObjectPtr>::const_iterator	i;
	double	result = 0;
	for (i = objects.begin(); i != objects.end(); i++) {
		result += (*i)->intensity(where);
	}
	return result;
}

} // namespace astro
