/*
 * Stars.h -- The Simulator needs to create artificial stars and star fields
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Stars_h
#define _Stars_h

#include <AstroTypes.h>

namespace astro {

/**
 * \brief Base class for all objects that con possibly end up in an image
 *
 * The common characteristic of all these objects is their position and
 * an intensity distribution 
 */
class StellarObject {
	Point	_position;
protected:
	double	distance(const Point& point) const { return point - _position; }
public:
	StellarObject(const Point& position) : _position(position) { }
	const Point&	position() const { return _position; }
	void	position(const Point& position) { _position = position; }
	virtual double	intensity(const Point& where) const = 0;
};

/**
 * \brief Star class
 *
 * Stars are pointlike objects, but the intensity distribution is a
 * gaussian.
 */
class Star : public StellarObject {
	double	_magnitude;
	double	_peak;
public:
	Star(const Point& position, double magnitude = 1)
		: StellarObject(position), _magnitude(magnitude) { }
	const double&	magnitude() const { return _magnitude; }
	void	magnitude(const double& magnitude);
	virtual double intensity(const Point& where) const;
};

/**
 * \brief Nebula class
 *
 * Nebulae are circular objects of uniform density
 */
class Nebula : public StellarObject {
	double	_radius;
	double	_density;
public:
	Nebula(const Point& center, double radius)
		: StellarObject(center), _radius(radius), _density(1) { }
	const double&	radius() const { return _radius; }
	void	radius(const double& radius) { _radius = radius; }
	const double&	density() const { return _density; }
	void	density(const double& density) { _density = density; }
	virtual double intensity(const Point& where) const;
};

typedef std::tr1::shared_ptr<StellarObject>	StellarObjectPtr;

/**
 * \brief Star fields
 *
 * A Star field is essentially a set of stellar objects, that are then
 * added together for the final image
 */
class StarField {
	std::vector<StellarObjectPtr>	objects;
	void	createStar(const ImageSize& size, int overshoot);
public:
	StarField(const ImageSize& size, int overshoot = 100,
		unsigned int nobjects = 100);
	void	addObject(StellarObjectPtr object);
	virtual double	intensity(const Point& where) const;
};

/**
 * \brief A functor to turn star fields into images
 *
 * This functor adds up the intensity distributions of all objects of
 * the star field. It also adds some artefacts of the camera like
 * thermal noise, or vignetting.
 */
template<typename P>
class StarCamera {
	ImageSize	_size;
public:
	StarCamera(const ImageSize& size) : _size(size) { }

	ImagePtr	operator()(const StarField& field) const {
		Image<P>	*image = new Image<P>(_size);
		double	scale = std::numeric_limits<P>::max();
		for (unsigned int x = 0; x < _size.width(); x++) {
			for (unsigned int y = 0; y < _size.height(); y++) {
				P	value = scale * field.intensity(Point(x, y));
				image->pixel(x, y) = value;
			}
		}
		return ImagePtr(image);
	}
};

} // namespace astro

#endif /* _Stars_h */
