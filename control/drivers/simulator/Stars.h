/*
 * Stars.h -- The Simulator needs to create artificial stars and star fields
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Stars_h
#define _Stars_h

#include <AstroTypes.h>
#include <AstroTransform.h>
#include <set>

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
	virtual std::string	toString() const { return _position.toString(); }
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
	Star(const Point& position, double magnitude = 0);
	const double&	magnitude() const { return _magnitude; }
	void	magnitude(const double& magnitude);
	virtual double intensity(const Point& where) const;
	virtual std::string	toString() const;
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
	virtual std::string	toString() const;
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
	const StellarObjectPtr&	operator[](size_t index) const {
		return objects[index];
	}
};

/**
 * \brief Base class for the star camera
 * 
 * The StarCamera template shares many functions that are not dependent
 * on the template argument type. To save code duplication, we collect them
 * in this base class.
 */
class StarCameraBase {
	ImageRectangle	_rectangle;
	Point	_translation;
	double	_alpha;
	double	_stretch;
	double	_dark;
	double	_noise;
	bool	_light;
	void	addHotPixel();
protected:
	double	noisevalue() const;
	std::set<ImagePoint>	hotpixels;
public:
	StarCameraBase(const ImageRectangle& rectangle)
		: _rectangle(rectangle), _alpha(0), _stretch(1),
		  _dark(0), _noise(0), _light(true) { }

	void	addHotPixels(unsigned int npixels);

	void	rectangle(const ImageRectangle& rectangle) {
		_rectangle = rectangle;
	}
	const ImageRectangle&	rectangle() const { return _rectangle; }

	const Point&	translation() const { return _translation; }
	void	translation(const Point& translation) {
		_translation = translation;
	}

	const double&	alpha() const { return _alpha; }
	void	alpha(const double& alpha) { _alpha = alpha; }

	const double&	stretch() const { return _stretch; }
	void	stretch(const double& stretch) { _stretch = stretch; }

	const double&	dark() const { return _dark; }
	void	dark(const double& dark) { _dark = dark; }

	const double&	noise() const { return _noise; }
	void	noise(const double& noise) { _noise = noise; }

	const bool&	light() const { return _light; }
	void	light(const bool& light) { _light = light; }
};

/**
 * \brief A functor to turn star fields into images
 *
 * This functor adds up the intensity distributions of all objects of
 * the star field. It also adds some artefacts of the camera like
 * thermal noise, or vignetting.
 */
template<typename P>
class StarCamera : public StarCameraBase {
public:
	StarCamera(const ImageRectangle& rectangle)
		: StarCameraBase(rectangle) { }

	ImagePtr	operator()(const StarField& field) const {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "imaging star field %s",
			field[0]->toString().c_str());
			
		// create the image
		ImageSize	size = rectangle().size();
		ImagePoint	origin = rectangle().origin();
		Image<P>	*image = new Image<P>(size);

		// compute a transform based on translation and rotation
		debug(LOG_DEBUG, DEBUG_LOG, 0, "translation = %s, alpha = %f",
			translation().toString().c_str(), alpha());
		astro::image::transform::Transform	transform(alpha(),
							-translation());

		// whether or not we should do the somewhat onerous noise
		// computation
		bool	do_noise = (noise() > 0);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s image",
			(light()) ? "light" : "dark");

		// now compute all the pixel values
		double	scale = std::numeric_limits<P>::max();
		for (unsigned int x = 0; x < size.width(); x++) {
			for (unsigned int y = 0; y < size.height(); y++) {
				// apply the transform to the current point
				Point	where(origin.x() + x, origin.y() + y);
				Point	p = transform(where);

				// compute the intensity
				double	value = (light())
					? stretch() * field.intensity(p) : 0;
				if (do_noise) {
					value += noisevalue();
				}
				value *= scale;

				// clamp the the range of the pixel type
				if (value > scale) {
					value = scale;
				}
				image->pixel(x, y) = value;
			}
		}

		// turn pixels hot
		std::set<ImagePoint>::const_iterator	i;
		for (i = hotpixels.begin(); i != hotpixels.end(); i++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "add hot pixel %s",
				i->toString().c_str());
			if (rectangle().contains(*i)) {
				image->pixel(i->x() - origin.x(),
						i->y() - origin.y()) = scale;
			}
		}

		// that's the image
		return ImagePtr(image);
	}
};

} // namespace astro

#endif /* _Stars_h */
