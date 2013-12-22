/*
 * Stars.h -- The Simulator needs to create artificial stars and star fields
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Stars_h
#define _Stars_h

#include <AstroTypes.h>
#include <AstroTransform.h>
#include <AstroCamera.h>
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
	astro::image::RGB<double>	_color;
protected:
	double	distance(const Point& point) const { return point - _position; }
public:
	StellarObject(const Point& position);
	const Point&	position() const { return _position; }
	void	position(const Point& position) { _position = position; }
	const astro::image::RGB<double>&	color() const { return _color; }
	void	color(const astro::image::RGB<double>& color) {
		_color = color;
	}
	virtual double	intensity(const Point& where) const = 0;
	double	intensityR(const Point& where) const;
	double	intensityG(const Point& where) const;
	double	intensityB(const Point& where) const;
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

typedef std::shared_ptr<StellarObject>	StellarObjectPtr;

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
	virtual double	intensityR(const Point& where) const;
	virtual double	intensityG(const Point& where) const;
	virtual double	intensityB(const Point& where) const;
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
	/**
	 * \brief Translation to be applied to the star field
	 */
	Point	_translation;
	/**
	 * \brief Rotation angle 
 	 */
	double	_alpha;
	/**
	 * \brief Factor by which to stretch the star field
 	 */
	double	_stretch;
	/**
	 * \brief Dark value
 	 */
	double	_dark;
	/**
	 * \brief Noise standard deviation
 	 */
	double	_noise;
	/**
	 * \brief Whether or not the camera shutter is open
	 */
	bool	_light;
	/**
	 * \brief Color
 	 */
	int	_color;
	/**
	 * \brief The outer radius if the image is out of focus
	 *
	 * Set this value to 0 to get focused images
	 */
	double	_radius;
	/**
	 * \brief Inner radius to simulate donuts
	 *
	 * reflector telescopes have images that show "donuts" when they
	 * are out of focus. This value describes the inner radius
	 */
	double	_innerradius;
	void	addHotPixel();
	/**
	 * \brief Binning mode to apply when exposing
	 */
	astro::camera::Binning	_binning;
	double	bin0(Image<double>& image, unsigned int x, unsigned int y) const;
	void	fill0(Image<double>& image, const ImagePoint& point,
			double fillvalue) const;
protected:
	void	addnoise(Image<double>& image) const;
	void	addhot(Image<double>& image, double hotvalue) const;
	void	rescale(Image<double>& image, double scale) const;
	void	bin(Image<double>& image) const;
	double	noisevalue() const;
	std::set<ImagePoint>	hotpixels;
public:
	StarCameraBase(const ImageRectangle& rectangle)
		: _rectangle(rectangle), _alpha(0), _stretch(1),
		  _dark(0), _noise(0), _light(true), _color(0), _radius(0),
		  _innerradius(0) { }

	void	addHotPixels(unsigned int npixels);

	void	rectangle(const ImageRectangle& rectangle) {
		_rectangle = rectangle;
	}
	const ImageRectangle&	rectangle() const { return _rectangle; }

	// accessors to the translation
	const Point&	translation() const { return _translation; }
	void	translation(const Point& translation) {
		_translation = translation;
	}

	// accessors for the rotation angle
	const double&	alpha() const { return _alpha; }
	void	alpha(const double& alpha) { _alpha = alpha; }

	// accessor for the stretch factor
	const double&	stretch() const { return _stretch; }
	void	stretch(const double& stretch) { _stretch = stretch; }

	// accessor for the dark value
	const double&	dark() const { return _dark; }
	void	dark(const double& dark) { _dark = dark; }

	// accessor for the noise standard deviation
	const double&	noise() const { return _noise; }
	void	noise(const double& noise) { _noise = noise; }

	// accessor for the shutter flag
	const bool&	light() const { return _light; }
	void	light(const bool& light) { _light = light; }

	// accessor for the color parameter
	const int&	color() const { return _color; }
	void	colorfactor(const int& color) { _color = color; }

	// accessor for the out of focus radius
	const double&	radius() const { return _radius; }
	void	radius(const double& radius) { _radius = radius; }

	// accessor for the inner radius for donuts
	const double&	innerradius() const { return _innerradius; }
	void	innerradius(const double& innerradius) {
		_innerradius = innerradius;
	}

	// accessor for the binning mode
	const astro::camera::Binning&	binning() const { return _binning; }
	void	binning(const astro::camera::Binning& binning) { _binning = binning; }

	// imaging operator
	Image<double>	*operator()(const StarField& field) const;
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
			
		// compute the image
		Image<double>	*rawimage = StarCameraBase::operator()(field);

		// bin the image,
		if (binning() != astro::camera::Binning()) {
			bin(*rawimage);
		}

		// now add all the local stuff, which depends on the camera,
		// not the star field
		double	scale = std::numeric_limits<P>::max();
		rescale(*rawimage, scale);

		// turn pixels hot, this must respsect the binning
		addhot(*rawimage, scale);

		// now convert the image into an image of the right pixel type
		// create the image
		ImageSize	size = rectangle().size() / binning();
		Image<P>	*image = new Image<P>(size);

		// fill in the data
		unsigned int	width = size.width();
		unsigned int	height = size.height();
		unsigned int	deltax = binning().getX();
		unsigned int	deltay = binning().getY();
		for (unsigned int x = 0; x < width; x++) {
			for (unsigned int y = 0; y < height; y++) {
				image->pixel(x, y)
					= rawimage->pixel(x * deltax, y * deltay);
			}
		}

		// remove the raw image, we no longer need it
		delete rawimage;

		// that's the image
		return ImagePtr(image);
	}
};

} // namespace astro

#endif /* _Stars_h */
