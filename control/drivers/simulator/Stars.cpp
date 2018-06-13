/*
 * Stars.cpp -- star implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Stars.h>
#include <AstroAdapter.h>
#include <Blurr.h>
#include <math.h>

using namespace astro::image;

namespace astro {

inline double	sqr(const double x) {
	return x * x;
}

/**
 * \brief Construct a new stellar object
 */
StellarObject::StellarObject(const Point& position) : _position(position) {
	_color = RGB<double>(1., 1., 1.);
}

/**
 * \brief Destroy the StellarObject
 */
StellarObject::~StellarObject() {
}

/**
 * \brief Extract red color value
 */
double	StellarObject::intensityR(const Point& where) {
	return _color.R * this->intensity(where);
}

/**
 * \brief Extract blue color value
 */
double	StellarObject::intensityB(const Point& where) {
	return _color.B * this->intensity(where);
}

/**
 * \brief Extract green color value
 */
double	StellarObject::intensityG(const Point& where) {
	return _color.G * this->intensity(where);
}

/**
 * \brief Create a new star
 */
Star::Star(const Point& position, double magnitude)
	: StellarObject(position) {
	this->magnitude(magnitude);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "new star at %s",
	//	position.toString().c_str());
}

Star::~Star() {
}

/**
 * \brief Intensity distribution for a star
 */
#define	AIRY_RADIUS	1.5
static const double	vpeak = (0.5 / AIRY_RADIUS);
double	Star::intensity(const Point& where) {
	double	d = distance(where);
	// short circuit far away points to improve speed
	if (d > 10) {
		return 0;
	}
	double	r = sqr(distance(where));
	double	v = 1;
	if (r > 0) {
		v = fabs(j1(r / AIRY_RADIUS) / r) / vpeak;
	}
	return _peak * v;
}

/**
 * \brief Magnitude setter method
 *
 * The magnitude also affects the peak value, so we ensure in the setter
 * that the _peak value is always consistent with the magnitude. Computing
 * the _peak is expensive, and doing it in the intensity method (where it
 * is needed) would slow image computation down.
 */
void	Star::magnitude(const double& magnitude) {
	_magnitude = magnitude;
	_peak = pow(10, -(magnitude / 2.5));
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "peak(%f) = %f", _magnitude, _peak);
}

/**
 * \brief String representation of a star
 */
std::string	Star::toString() const {
	return stringprintf("star %.2f@%s", _magnitude,
		StellarObject::toString().c_str());
}

/**
 * \brief Nebula intensity distribution: circular disk
 */
double	Nebula::intensity(const Point& where) {
	return (distance(where) > _radius) ? 0 : _density;
}

/**
 * \brief String representation of a nebula
 */
std::string	Nebula::toString() const {
	return stringprintf("nebula %.2fx%.f@", _density, _radius,
		StellarObject::toString().c_str());
}

} // namespace astro
