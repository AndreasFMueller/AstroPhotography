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
#include <typeinfo>

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
	SkyWindow();
	bool	contains(const RaDec& position) const;
	std::pair<double, double>	decinterval() const;
	Angle	leftra() const;
	Angle	rightra() const;
	Angle	topdec() const;
	Angle	bottomdec() const;
	virtual std::string	toString() const;
	static SkyWindow	all;
	void	addMetadata(ImageBase& image) const;
	static SkyWindow	hull(const RaDec& center, const Angle& rawidth,
					const Angle& decheight);
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
	void	mag(float m) { _mag = m; }
};

class Star;
typedef std::shared_ptr<Star>	StarPtr;

/**
 * \brief Star base class
 *
 * Stars are celestial objects that in addition have a magnitude
 */
class Star : public CelestialObject {
	std::string	_name;
	std::string	_longname;
public:
	const std::string&	name() const { return _name; }
	const std::string&	longname() const { return _longname; }
	void	longname(const std::string& l) { _longname = l; }
private:
	char	_catalog;
public:
	char	catalog() const { return _catalog; }
	void	catalog(const char c) { _catalog = c; }
private:
	uint64_t	_catalognumber;
public:
	uint64_t	catalognumber() const { return _catalognumber; }
	void	catalognumber(uint64_t c) { _catalognumber = c; }

private:
	char	_duplicate;
	std::string	_duplicatename;
public:
	bool	isDuplicate() const;
	char	duplicateCatalog() const;
	const std::string& duplicatename() const;
	void	setDuplicate(char catalog, const std::string& name);

	// constructors
	Star(const std::string& name) : _name(name) {
		_mag = 0;
		_duplicate = '\0';
	}
	virtual ~Star() { }
	virtual std::string	toString() const;
};

/**
 * \brief Deep Sky objects
 */
class DeepSkyObject : public CelestialObject {
public:
	DeepSkyObject() { _mag = 0; }
	virtual ~DeepSkyObject() { }
	std::string	name;
	std::string	constellation;
	typedef enum { Galaxy, OpenCluster, GlobularCluster, BrightNebula,
			PlanetaryNebula, ClusterNebulosity, Asterism,
			Knot, TripleStar, DoubleStar, SingleStar, Uncertain,
			Unidentified, Nonexistent, PlateDefect } object_class;
	object_class	classification;
	static std::string	classification2string(object_class);
	static object_class	string2classification(const std::string&);
	TwoAngles	size;
	Angle		azimuth;
	std::string	toString() const;
};

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


class IteratorImplementation;
typedef std::shared_ptr<IteratorImplementation>	IteratorImplementationPtr;

/**
 * \brief Catalog Iterator
 *
 * The CatalogIterator class is a wrapper that hides implementation details
 * from users of the catalogs
 */
class CatalogIterator {
	IteratorImplementationPtr	_implementation;
public:
	IteratorImplementationPtr	implementation() const {
		return _implementation;
	}
private:
	void	check() const;
public:
	CatalogIterator(IteratorImplementationPtr implementation);
	CatalogIterator();
	CatalogIterator(const CatalogIterator& other);
	CatalogIterator&	operator=(const CatalogIterator& other);
	Star	operator*();
	bool	operator==(const CatalogIterator& other) const;
	bool	operator!=(const CatalogIterator& other) const;
	CatalogIterator	operator++();
	std::string	toString() const;
	bool	isEnd() const;
};

class Catalog;
typedef std::shared_ptr<Catalog>	CatalogPtr;

/**
 * \brief A collection of star catalogs
 */
class Catalog {
	// prevent copying of Catalog objects
	Catalog(const Catalog& other);
	Catalog&	operator=(const Catalog& other);
protected:
	std::string	backendname;
public:
	// constructors
	Catalog() { }
	virtual ~Catalog();

	// naming the catalog
	const std::string&	name() const { return backendname; }

	// find and add individual stars
	virtual Star	find(const std::string& name) = 0;

	// types for search results
	typedef	std::set<Star>	starset;
	typedef std::shared_ptr<starset>	starsetptr;

	// start to find the all stars in an a sky window
	virtual starsetptr	find(const SkyWindow& window,
					const MagnitudeRange& magrange) = 0;
	virtual CatalogIterator	findIter(const SkyWindow& window,
					const MagnitudeRange& magrange);
	// some information about the size of the catalog
	virtual unsigned long	numberOfStars() = 0;

	// iterator interface for traversing the catalog
	virtual CatalogIterator	begin();
	CatalogIterator	end();
};

Catalog::starsetptr	precess(const Precession& precession,
				Catalog::starsetptr stars);

/**
 * \brief Factory class to retrieve a Catalog
 */
class CatalogFactory {
public:
	typedef enum {
		BSC = 0,
		Tycho2 = 1,
		Hipparcos = 2,
		Ucac4 = 3,
		Combined = 4,
		Database = 5
	} BackendType;
	static CatalogPtr	get(BackendType type,
					const std::string& parameter);
	static CatalogPtr	get(BackendType type);
	static CatalogPtr	get();
};

/**
 * \brief DeepSkyCatalog
 */
class DeepSkyCatalog {
protected:
	std::string	_basedir;
public:
	DeepSkyCatalog(const std::string& basedir) : _basedir(basedir)  { }
	virtual ~DeepSkyCatalog() { }

	typedef std::set<DeepSkyObject>	deepskyobjectset;
	typedef std::shared_ptr<deepskyobjectset> deepskyobjectsetptr;

	virtual deepskyobjectsetptr	find(const SkyWindow&) = 0;
	virtual DeepSkyObject	find(const std::string& name) = 0;
	virtual	std::set<std::string>	findLike(const std::string& name) = 0;
};
typedef std::shared_ptr<DeepSkyCatalog>	DeepSkyCatalogPtr;

/**
 * \brief A Factory to build deep sky catalogs
 */
class DeepSkyCatalogFactory {
	std::string	_basedir;
public:
	typedef enum deepskycatalog_e { Messier, NGCIC } deepskycatalog_t;
	DeepSkyCatalogFactory(const std::string& basedir) : _basedir(basedir) { }
	DeepSkyCatalogFactory();
	DeepSkyCatalogPtr	get(deepskycatalog_t ct);
};

/*
 * \brief Outline of a DeepSkyObject
 */
class Outline : public std::list<astro::RaDec> {
	std::string	_name;
public:
	const std::string&	name() const { return _name; }
	void	name(const std::string& name) { _name = name; }
	Outline(const std::string& name) : _name(name) { }
	std::string	toString() const;
};

/**
 * \brief Build a catalog of outlines
 */
class OutlineCatalog {
	std::map<std::string, Outline>	_outlinemap;
	void	parse(const std::string& directory);
public:
	OutlineCatalog();
	OutlineCatalog(const std::string& directory);
	bool	has(const std::string& name) const;
	Outline	find(const std::string& name) const;
};

} // namespace catalog
} // namespace astro

#endif /* _AstroCatalog_h */
