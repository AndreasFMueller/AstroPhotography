/*
 * AstroChart.h -- Using Star catalogs to create charts
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroChart_h
#define _AstroChart_h

#include <AstroCoordinates.h>
#include <AstroCatalog.h>

namespace astro {
namespace catalog {

/**
 * \brief ImageGeometry
 */
class ImageGeometry {
	ImageSize	_size;
	double	_pixelsize;
	double	_focallength;
public:
	ImageGeometry(const ImageSize& size, double pixelsize,
		double focallength) : _size(size), _pixelsize(pixelsize),
			_focallength(focallength)  { }
	ImageGeometry(const ImageBase& image);
	const ImageSize&	size() const { return _size; }
	double	pixelsize() const { return _pixelsize; }
	double	focallength() const { return _focallength; }
	void	addMetadata(ImageBase& image) const;
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
	bool	contains(const RaDec& point) const;
	astro::Point	map(const RaDec& where) const;
	astro::Point	map2(const RaDec& where) const;
	SkyWindow	containedin() const;
	RaDec	inverse(const astro::Point& p) const;
	void	addMetadata(ImageBase& image) const;
};

/**
 * \brief Chart abstraction
 *
 * A Chart consists of an image and SkyRectangle that defines the
 * coordinate system.
 */
class Chart {
private:
	SkyRectangle	_rectangle;
	ImagePtr	_imageptr;
public:
	Chart(const SkyRectangle rectangle, const ImagePtr image)
		: _rectangle(rectangle), _imageptr(image) { }
	const SkyRectangle	rectangle() const { return _rectangle; }
	const ImagePtr	image() const { return _imageptr; }
};

/**
 * \brief Chart abstraction
 *
 * Class to produce charts for sets of stars. 
 */
class ChartFactory {
private:
	Image<float>	*_image;
	ImagePtr	_imageptr;
public:
	const astro::image::ImageSize	size() const { return _image->size(); }
private:
	double	_scale;
public:
	double	scale() const { return _scale; }
	void	scale(double s) { _scale = s; }
private:
	bool	_logarithmic;
public:
	bool	logarithmic() const { return _logarithmic; }
	void	logarithmic(bool l) { _logarithmic = l; }
private:
	double	_maxradius;
public:
	unsigned int	maxradius() const { return _maxradius; }
	void	maxradius(unsigned int m) { _maxradius = m; }
protected:
	virtual double	pointspreadfunction(double r, double mag) const;
private:
	SkyRectangle	_rectangle;
	void	draw(const Star& star);
protected:
	double	_focallength;
	double	_pixelsize;
public:
	ChartFactory(const astro::image::ImageSize& size, const RaDec& center,
		double focallength, double pixelsize);
	~ChartFactory();
	Chart	chart() const { return Chart(_rectangle, _imageptr); }
	SkyWindow	getWindow() const;
	astro::Point	point(const RaDec& star) const;
	void	draw(const Catalog::starset& star);
	void	draw(const Catalog::starsetptr star);
	void	clear();
};

/**
 * \brief DiffractionChart
 *
 * Chart built based on the assumption that star images are only modified
 * by diffraction into an Airy disk. To compute the diffraction pattern,
 * the aperture must be known. This model is only useful for extremely
 * good atmospheric conditions (perfect seeing), and small apertures, as
 * for large apertures the size of the airy disk is probably on the order
 * of the pixels of the camera.
 */
class DiffractionChartFactory : public ChartFactory {
	double	_aperture;
	double	_xfactor;
public:
	double	aperture() const { return _aperture; }
	void	aperture(double a);
private:
	virtual double	pointspreadfunction(double r, double mag) const;
public:
	DiffractionChartFactory(const astro::image::ImageSize& size,
		const RaDec& center, double focallength, double pixelsize);
};

/**
 * \brief TurbulenceChart
 *
 * The TurbulenceChart assumes that the main reason stars are not points is
 * seeing. This is probably what you one expects in near civilization 
 * viewing conditions. The FWHM of the seeing disk is given by the turbulence
 * parameter.
 */
class TurbulenceChartFactory : public ChartFactory {
	double	_turbulence;
public:
	double	turbulence() const { return _turbulence; }
	void	turbulence(double t) { _turbulence = t; }
private:
	virtual double	pointspreadfunction(double r, double mag) const;
public:
	TurbulenceChartFactory(const astro::image::ImageSize& size,
		const RaDec& center, double focallength, double pixelsize);
};

/**
 * \brief Normalize an image
 */
class ImageNormalizer {
public:
};

} // namespace catalog
} // namespace astro

#endif /* _AstroChart_h */
