/*
 * Stars.cpp -- star implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Stars.h>

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
 * \brief Extract red color value
 */
double	StellarObject::intensityR(const Point& where) const {
	return _color.R * this->intensity(where);
}

/**
 * \brief Extract blue color value
 */
double	StellarObject::intensityB(const Point& where) const {
	return _color.B * this->intensity(where);
}

/**
 * \brief Extract green color value
 */
double	StellarObject::intensityG(const Point& where) const {
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

/**
 * \brief Intensity distribution for a star
 */
double	Star::intensity(const Point& where) const {
	double	d = distance(where);
	// short circuit far away points to improve speed
	if (d > 10) {
		return 0;
	}
	return _peak * exp(-sqr(distance(where) / (2)));
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
	_peak = 10 * exp(-0.9 * _magnitude);
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
double	Nebula::intensity(const Point& where) const {
	return (distance(where) > _radius) ? 0 : _density;
}

/**
 * \brief String representation of a nebula
 */
std::string	Nebula::toString() const {
	return stringprintf("nebula %.2fx%.f@", _density, _radius,
		StellarObject::toString().c_str());
}

/**
 * \brief Create a new star field
 *
 * \param size		The image field size
 * \param overshoot	how many pixels to add on each side of the frame
 * \param nobjects	number of stars to generate
 */
StarField::StarField(const ImageSize& size, int overshoot,
	unsigned int nobjects) {
	for (unsigned int i = 0; i < nobjects; i++) {
		createStar(size, overshoot);
	}
}

/**
 * \brief Create a random star
 *
 * Stars are evenly distributed in the rectangle formed by adding
 * overshoot to the camera frame on each side. The magnitudes follow
 * a power distribution, which may not be entirely accurate, but is
 * a sufficiently good model for this simulation.
 */
void	StarField::createStar(const ImageSize& size, int overshoot) {
	int	x = (random() % (size.width() + 2 * overshoot)) - overshoot;
	int	y = (random() % (size.height() + 2 * overshoot)) - overshoot;
	// create magnitudes with a power distribution
	double	magnitude = log2(8 + (random() % 56)) - 3;

	StellarObject	*newstar = new Star(Point(x, y), magnitude);

	// create color
	int	colorcode = random() % 8;
	double	red = (colorcode & 4) ? 1.0 : 0.8;
	double	green = (colorcode & 2) ? 1.0 : 0.8;
	double	blue = (colorcode & 1) ? 1.0 : 0.8;
	RGB<double>	color(red, green, blue);
	newstar->color(color);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "color: %.2f/%.2f/%.2f",
	//	red, green, blue);
	
	// the new star
	addObject(StellarObjectPtr(newstar));
}

/**
 * \brief Add a new stellar object
 *
 * This method accepts stars or nebulae
 */
void	StarField::addObject(StellarObjectPtr object) {
	objects.push_back(object);
}

/**
 * \brief Compute cumulated intensity for all objects in the star field
 */
double	StarField::intensity(const Point& where) const {
	std::vector<StellarObjectPtr>::const_iterator	i;
	double	result = 0;
	for (i = objects.begin(); i != objects.end(); i++) {
		result += (*i)->intensity(where);
	}
	return result;
}

/**
 * \brief Compute cumulated intensity for all objects in the star field
 */
double	StarField::intensityR(const Point& where) const {
	std::vector<StellarObjectPtr>::const_iterator	i;
	double	result = 0;
	for (i = objects.begin(); i != objects.end(); i++) {
		result += (*i)->intensityR(where);
	}
	return result;
}

/**
 * \brief Compute cumulated intensity for all objects in the star field
 */
double	StarField::intensityG(const Point& where) const {
	std::vector<StellarObjectPtr>::const_iterator	i;
	double	result = 0;
	for (i = objects.begin(); i != objects.end(); i++) {
		result += (*i)->intensityG(where);
	}
	return result;
}

/**
 * \brief Compute cumulated intensity for all objects in the star field
 */
double	StarField::intensityB(const Point& where) const {
	std::vector<StellarObjectPtr>::const_iterator	i;
	double	result = 0;
	for (i = objects.begin(); i != objects.end(); i++) {
		result += (*i)->intensityB(where);
	}
	return result;
}

/**
 * \brief compute a random point and add it as a hot pixel position
 */
void	StarCameraBase::addHotPixel() {
	int	x = random() % rectangle().size().width();
	int	y = random() % rectangle().size().height();
	ImagePoint	hotpixel(x, y);
	hotpixels.insert(hotpixel);
}

/**
 * \brief Add a number of hot pixels
 */
void	StarCameraBase::addHotPixels(unsigned int npixels) {
	unsigned int	limit = hotpixels.size() + npixels;
	while (hotpixels.size() < limit) {
		addHotPixel();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera has now %d hot pixels",
		hotpixels.size());
}

/**
 * \brief Compute inverse error function using Newton's algorithm
 *
 * The error function in the C Library is defined as
 *
 * erf(x) = 2/sqrt(pi)*integral from 0 to x of exp(-t*t) dt.
 *
 * The derivative of erf(x) is of course
 *
 * erf'(x) = 2/sqrt(pi) exp(-t*t)
 *
 * 
 */
#define	epsilon	0.000001
#define	maxiterations	10

static double	inverf(double y) {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "y = %f", y);
	double	x = y - 0.5;
	double	m = 2 / sqrt(M_PI);
	double	delta = 1;
	int	counter = 0;
	while ((counter++ < maxiterations) && (delta > epsilon)) {
		delta = (erf(x) - y) / (m * exp(-x * x));
		x -= delta;
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "x = erf(y) = %f", x);
	}
	return x;
}

double	StarCameraBase::noisevalue() const {
	double	x = random() / (double)0xffffffff;
	return _noise * inverf(x);
}

} // namespace astro
