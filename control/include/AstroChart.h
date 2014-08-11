/*
 * AstroChart.h -- Using Star catalogs to create charts
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroChart_h
#define _AstroChart_h

#include <AstroCoordinates.h>
#include <AstroCatalog.h>
#include <AstroProjection.h>

namespace astro {
namespace catalog {

/**
 * \brief ImageGeometry
 */
class ImageGeometry : public ImageSize {
	double	_pixelsize;
	double	_focallength;
public:
	ImageGeometry(const ImageSize& size, double focallength,
		double pixelsize) : ImageSize(size), _pixelsize(pixelsize),
			_focallength(focallength)  { }
	ImageGeometry(const ImageBase& image);
	double	pixelsize() const { return _pixelsize; }
	double	focallength() const { return _focallength; }
	void	addMetadata(ImageBase& image) const;
	Angle	rawidth() const;
	Angle	decheight() const;
	virtual std::string	toString() const;
	Point	coordinates(const Point& a) const;
};

/**
 * \brief A rectangle on the sky
 */
class SkyRectangle : public SkyWindow {
	UnitVector	direction;
	UnitVector	rightvector;
	UnitVector	upvector;
	double	uplimit;
	double	rightlimit;
	void	setup();
public:
	SkyRectangle();
	SkyRectangle(const SkyWindow& window);
	SkyRectangle(const ImageBase& image);
	SkyRectangle(const RaDec& center, const ImageGeometry& geometry);
	bool	contains(const RaDec& point) const;
	astro::Point	map(const RaDec& where) const;
	astro::Point	map2(const RaDec& where) const;
	astro::Point	point(const ImageSize& size, const RaDec& where) const;
	SkyWindow	containedin() const;
	RaDec	inverse(const astro::Point& p) const;
	void	addMetadata(ImageBase& image) const;
};

class ChartFactory;
/**
 * \brief Chart abstraction
 *
 * A Chart consists of an image and SkyRectangle that defines the
 * coordinate system.
 */
class Chart {
private:
	SkyRectangle	_rectangle;
	ImageSize	_size;
	Image<double>	*_image;
	ImagePtr	_imageptr;
public:
	Chart(const SkyRectangle rectangle, const ImageSize& size);
	const SkyRectangle&	rectangle() const { return _rectangle; }
	const ImageSize&	size() const { return _size; }
	const ImagePtr	image() const { return _imageptr; }
friend class ChartFactory;
};

/**
 * \brief Point spread functions
 */
class PointSpreadFunction {
public:
	virtual double	operator()(double r, double mag) const = 0;
};

/**
 * \brief Point spread function the just turns a star into a circle
 */
class CirclePointSpreadFunction : public PointSpreadFunction {
	double	_maxradius;
public:
	CirclePointSpreadFunction(double maxradius) : _maxradius(maxradius) { }
	virtual double	operator()(double r, double mag) const;
};

/**
 * \brief The point spread function for an image dominated by diffraction
 */
class DiffractionPointSpreadFunction : public PointSpreadFunction {
	double	_aperture;
	double	_xfactor;
public:
	DiffractionPointSpreadFunction(const ImageGeometry& geometry,
		double aperture);
	virtual double	operator()(double r, double mag) const;
};

/**
 * \brief The point spread function for an image dominated by turbulence
 */
class TurbulencePointSpreadFunction : public PointSpreadFunction {
	double	_turbulence;
public:
	TurbulencePointSpreadFunction(double turbulence = 2.)
		: _turbulence(turbulence) {
	}
	virtual double	operator()(double r, double mag) const;
};

/**
 * \brief Chart abstraction
 *
 * Class to produce charts for sets of stars. 
 */
class ChartFactory {
// parameters valid for all images
private:
	Catalog&	_catalog;
	PointSpreadFunction&	pointspreadfunction;
private:
	double	_limit_magnitude;
public:
	double	limit_magnitude() const { return _limit_magnitude; }
	void	limit_magnitude(double l) { _limit_magnitude = l; }
private:
	double	_scale;
public:
	double	scale() const { return _scale; }
	void	scale(double s) { _scale = s; }
private:
	double	_maxradius;
public:
	unsigned int	maxradius() const { return _maxradius; }
	void	maxradius(unsigned int m) { _maxradius = m; }
private:
	bool	_logarithmic;
public:
	bool	logarithmic() const { return _logarithmic; }
	void	logarithmic(bool l) { _logarithmic = l; }
public:
	// constructors
	ChartFactory(Catalog& catalog, PointSpreadFunction& psf,
		double limit_magnitude = 16,
		double scale = 1, double maxradius = 7,
		bool logarithmic = false)
		: _catalog(catalog), pointspreadfunction(psf),
		  _limit_magnitude(limit_magnitude),
		  _scale(scale), _maxradius(maxradius),
		  _logarithmic(logarithmic) {
	}
	~ChartFactory();

	// functions needed to produce a chart
	Chart	chart(const RaDec& center, const ImageGeometry& geometry);
private:
	void	draw(Image<double>& image, const SkyRectangle& rectangle,
			const Catalog::starset& star);
	void	draw(Image<double>& image, const SkyRectangle& rectangle,
			const Catalog::starsetptr star);
	void	draw(Image<double>& image, const SkyRectangle& rectangle,
			const Star& star);
};

/**
 * \brief Normalize an image
 *
 * Find a projection 
 */
class ImageNormalizer {
	ChartFactory&	_factory;
public:
	ImageNormalizer(ChartFactory& factory);
	RaDec	operator()(astro::image::ImagePtr image,
			astro::image::transform::Projection& projection);
};

} // namespace catalog
} // namespace astro

#endif /* _AstroChart_h */
