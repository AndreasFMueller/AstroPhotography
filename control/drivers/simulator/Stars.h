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
	double	distance(const Point& point) const { return astro::distance(point, _position); }
public:
	StellarObject(const Point& position);
	virtual ~StellarObject();
	StellarObject(const StellarObject&) = delete;
	StellarObject&	operator=(const StellarObject&) = delete;
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
	virtual std::string	toString() const {
		return _position.toString();
	}
	void	transform(const image::transform::Transform& transform);
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
	virtual ~Star();
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
	double	radius() const { return _radius; }
	void	radius(const double& radius) { _radius = radius; }
	double	density() const { return _density; }
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
	ImageSize	_size;
	int	_overshoot;
	unsigned long	_seed;
	unsigned int	_nobjects;
	std::mutex	_mutex;
public:
	int	overshoot() const { return _overshoot; }
	const ImageSize&	size() const { return _size; }
public:
	StarField(const ImageSize& size, int overshoot = 100,
		unsigned int nobjects = 100);
	StarField(const StarField&) = delete;
	StarField&	operator=(const StarField&) = delete;
	void	clear();
	void	rebuild(unsigned long seed);
	void	rebuild(const RaDec& radec);
	void	addObject(StellarObjectPtr object);
	virtual double	intensity(const Point& where) const;
	virtual double	intensityR(const Point& where) const;
	virtual double	intensityG(const Point& where) const;
	virtual double	intensityB(const Point& where) const;
	size_t	nObjects() const { return objects.size(); }
	StellarObjectPtr	operator[](size_t index) const;
	void	transform(const image::transform::Transform& transform);
};

/**
 * \brief Base class for the star camera
 * 
 * The StarCamera template shares many functions that are not dependent
 * on the template argument type. To save code duplication, we collect them
 * in this base class.
 */
class StarCameraBase {
public:
	typedef enum { STARS, PLANET, SUN } content_type;
protected:
	content_type	_content;
public:
	content_type	content() const { return _content; }
	void	content(content_type c) { _content = c; }
private:
	ImageRectangle	_rectangle;
	/**
	 * \brief Translation to be applied to the star field
	 */
	Point	_translation;
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
	astro::image::Binning	_binning;
	double	bin0(Image<double>& image, int x, int y) const;
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
	StarCameraBase(const ImageRectangle& rectangle);
	StarCameraBase(const StarCameraBase&) = delete;
	StarCameraBase&	operator=(const StarCameraBase&) = delete;

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

	// accessor for the stretch factor
	double	stretch() const { return _stretch; }
	void	stretch(double stretch) { _stretch = stretch; }

	// accessor for the dark value
	double	dark() const { return _dark; }
	void	dark(double dark) { _dark = dark; }

	// accessor for the noise standard deviation
	double	noise() const { return _noise; }
	void	noise(double noise);

	// accessor for the shutter flag
	bool	light() const { return _light; }
	void	light(bool light) { _light = light; }

	// accessor for the color parameter
	int	color() const { return _color; }
	void	colorfactor(int color) { _color = color; }

	// accessor for the out of focus radius
	double	radius() const { return _radius; }
	void	radius(double radius) { _radius = radius; }

	// accessor for the inner radius for donuts
	double	innerradius() const { return _innerradius; }
	void	innerradius(double innerradius) {
		_innerradius = innerradius;
	}

	// accessor for the binning mode
	const astro::image::Binning&	binning() const { return _binning; }
	void	binning(const astro::image::Binning& binning) {
			_binning = binning;
}

	// imaging operator
private:
	void	addStarIntensity(Image<double>& image,
			StellarObjectPtr star,
			const Point& shift) const;

	void	addStarIntensities(Image<double>& image,
			const StarField& field,
			const Point& shift) const;

	void	addBodyIntensity(Image<double>& image,
			const Point& shift, int radius) const;

	void	addSunIntensity(Image<double>& image,
			const Point& shift) const;

	void	addPlanetIntensity(Image<double>& image,
			const Point& shift) const;
protected:
	Image<double>	*doubleImage(StarField& field);
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
	ImagePtr	operator()(StarField& field);
};

template<typename P>
ImagePtr	StarCamera<P>::operator()(StarField& field) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "apply camera to field");
//	try {
//		debug(LOG_DEBUG, DEBUG_LOG, 0, "imaging star field %s",
//			field[0]->toString().c_str());
//	} catch (const std::exception& x) {
//		debug(LOG_ERR, DEBUG_LOG, 0, "exception: %s", x.what());
//	}
		
	// compute the image
	Image<double>	*rawimage = StarCameraBase::doubleImage(field);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new image created");

	// bin the image,
	if (binning() != astro::image::Binning()) {
		bin(*rawimage);
	}

	// now add all the local stuff, which depends on the camera,
	// not the star field
	double	scale = std::numeric_limits<P>::max();
	rescale(*rawimage, scale / 2);

	// turn pixels hot, this must respsect the binning
	addhot(*rawimage, scale);

	// now convert the image into an image of the right pixel type
	// create the image
	ImageSize	size = rectangle().size() / binning();
	Image<P>	*image = new Image<P>(size);

	// fill in the data
	int	width = size.width();
	int	height = size.height();
	int	deltax = binning().x();
	int	deltay = binning().y();
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			double	v = rawimage->pixel(x * deltax, y * deltay);
			image->pixel(x, y) = v;
		}
	}

	// remove the raw image, we no longer need it
	delete rawimage;

	// that's the image
	return ImagePtr(image);
}

} // namespace astro

#endif /* _Stars_h */
