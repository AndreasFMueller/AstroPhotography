/*
 * AstroCatalog.h -- Generic star catalog classes
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroCatalog_h
#define _AstroCatalog_h

#include <AstroCoordinates.h>
#include <set>
#include <limits.h>
#include <string>
#include <AstroImage.h>
#include <AstroTypes.h>

namespace astro {
namespace catalog {

/**
 * \brief Window on the sky, used to select stars from the catalog
 */
class SkyWindow {
	RaDec	_center;
public:
	const RaDec&	center() const { return _center; }
private:
	Angle	_rawidth;
	Angle	_decheight;
public:
	const Angle&	rawidth() const { return _rawidth; }
	const Angle&	decheight() const { return _decheight; }
public:
	SkyWindow(const RaDec& center, const Angle& rawidth,
		const Angle& decheight);
	bool	contains(const RaDec& position) const;
	std::pair<double, double>	decinterval() const;
	Angle	leftra() const;
	Angle	rightra() const;
	Angle	bottomdec() const;
	virtual std::string	toString() const;
	static SkyWindow	all;
};

/**
 * \brief Celestial objects have position and proper motion
 */
class CelestialObject : public RaDec {
protected:
	RaDec	_pm; // proper motion in ra/yr dec/yr
public:
	const RaDec&	pm() const { return _pm; }
	RaDec&	pm() { return _pm; }
	RaDec	position(const double epoch) const;
};

/**
 * \brief Star base class
 *
 * Stars are celestial objects that in addition have a magnitude
 */
class Star : public CelestialObject {
protected:
	float	_mag;
public:
	const float&	mag() const { return _mag; }
	float&	mag() { return _mag; }
	Star() { _mag = 0; }
	std::string	toString() const;
};

class CatalogBackend;
typedef std::shared_ptr<CatalogBackend>	CatalogBackendPtr;

/**
 * \brief A collection of star catalogs
 */
class Catalog {
	CatalogBackendPtr	backend;
public:
	Catalog(const std::string& filename);
	typedef	std::set<Star>	starset;
	typedef std::shared_ptr<starset>	starsetptr;
	starsetptr	find(const SkyWindow& window,
				double minimum_magnitude);
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
	void	draw(const std::set<Star>& star);
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

#endif /* _AstroCatalog_h */
