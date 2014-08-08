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
	SkyWindow(const ImageBase& image);
	bool	contains(const RaDec& position) const;
	std::pair<double, double>	decinterval() const;
	Angle	leftra() const;
	Angle	rightra() const;
	Angle	bottomdec() const;
	virtual std::string	toString() const;
	static SkyWindow	all;
	void	addMetadata(ImageBase& image) const;
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
protected:
	float	_mag;
public:
	const float&	mag() const { return _mag; }
	float&	mag() { return _mag; }
};

/**
 * \brief Star base class
 *
 * Stars are celestial objects that in addition have a magnitude
 */
class Star : public CelestialObject {
	std::string	_name;
public:
	char	catalog;
	uint32_t	catalognumber;
	Star(const std::string& name) : _name(name) { _mag = 0; }
	const std::string	name() const { return _name; }
	std::string	toString() const;
};

/**
 * \brief Deep Sky objects
 */
class DeepSkyObject : public CelestialObject {
public:
	DeepSkyObject() { _mag = 0; }
	std::string	name;
	std::string	constellation;
	typedef enum { Galaxy, OpenCluster, GlobularCluster, BrightNebula,
			PlanetaryNebula, ClusterNebulosity, Asterism,
			Knot, TripleStar, DoubleStar, SingleStar, Uncertain,
			Unidentified, Nonexistent, PlateDefect } object_class;
	object_class	classification;
	Angle	size;
	std::string	toString() const;
};

class CatalogBackend;
typedef std::shared_ptr<CatalogBackend>	CatalogBackendPtr;

/**
 * \brief A class to encode a magnitude range
 */
class MagnitudeRange : std::pair<float, float> {
public:
	MagnitudeRange(float brightest, float faintest)
		: std::pair<float, float>(brightest, faintest) {
	}
	float	brightest() const { return first; }
	float&	brightest() { return first; }
	float	faintest() const { return second; }
	float&	faintest() { return second; }
	bool	contains(float mag) const {
		return (brightest() <= mag) && (mag <= faintest());
	}
	bool	empty() const { return (first == second); }
	std::string	toString() const;
};

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
				const MagnitudeRange& magrange);
	Star	find(const std::string& name);
};

} // namespace catalog
} // namespace astro

#endif /* _AstroCatalog_h */
