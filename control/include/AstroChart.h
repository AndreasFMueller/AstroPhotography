/*
 * AstroChart.h -- Using 
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
 * \brief A rectangle on the sky
 */
class SkyRectangle : public SkyWindow {
	UnitVector	direction;
	UnitVector	rightvector;
	UnitVector	upvector;
	double	uplimit;
	double	rightlimit;
public:
	SkyRectangle();
	SkyRectangle(const SkyWindow& window);
	bool	contains(const RaDec& point) const;
	astro::Point	map(const RaDec& where) const;
	astro::Point	map2(const RaDec& where) const;
	SkyWindow	containedin() const;
};

/**
 * \brief Chart abstraction
 *
 * class to produce charts for sets of stars
 */
class Chart {
private:
	Image<float>	*_image;
	ImagePtr	_imageptr;
public:
	ImagePtr	image() const { return _imageptr; }
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
	Chart(const astro::image::ImageSize& size, const RaDec& center,
		double focallength, double pixelsize);
	~Chart();
	SkyWindow	getWindow() const;
	astro::Point	point(const RaDec& star) const;
	void	draw(const Catalog::starset& star);
	void	draw(const Catalog::starsetptr star);
};

/**
 * \brief DiffractionChart
 *
 * Chart built based on the assumption that star images are only modified
 * by diffraction into an Airy disk
 */
class DiffractionChart : public Chart {
	double	_aperture;
	double	_xfactor;
public:
	double	aperture() const { return _aperture; }
	void	aperture(double a);
private:
	virtual double	pointspreadfunction(double r, double mag) const;
public:
	DiffractionChart(const astro::image::ImageSize& size,
		const RaDec& center, double focallength, double pixelsize);
};

/**
 * \brief TurbulenceChart
 */
class TurbulenceChart : public Chart {
	double	_turbulence;
public:
	double	turbulence() const { return _turbulence; }
	void	turbulence(double t) { _turbulence = t; }
private:
	virtual double	pointspreadfunction(double r, double mag) const;
public:
	TurbulenceChart(const astro::image::ImageSize& size,
		const RaDec& center, double focallength, double pixelsize);
};

} // namespace catalog
} // namespace astro

#endif /* _AstroChart_h */
