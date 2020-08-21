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
#include <array>

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
 * \brief LightWeightStar class as the smallest class useful for stars
 */
class LightWeightStar : public RaDec {
protected:
	float	_mag;
public:
	const float&	mag() const { return _mag; }
	void	mag(float m) { _mag = m; }
	LightWeightStar(const RaDec& position, float mag = 0)
		: RaDec(position), _mag(mag) { }
	LightWeightStar() { _mag = 0; }
};

/**
 * \brief a StarTile contains light weight stars in a small RA/DEC rectangle
 *
 * StarTiles are intended to improve the retrieval of large sets of stars
 * for the sky display widget
 */
class StarTile : public std::vector<LightWeightStar> {
	SkyWindow	_window;
public:
	StarTile(const SkyWindow& window) : _window(window) { }
	StarTile(const SkyWindow& window, size_t size)
		: std::vector<LightWeightStar>(size), _window(window) { }
};

typedef std::shared_ptr<StarTile>	StarTilePtr;

/**
 * \brief Celestial objects have proper motion in addition to position and mag
 */
class CelestialObject : public LightWeightStar {
protected:
	RaDec	_pm; // proper motion in ra/yr dec/yr
public:
	const RaDec&	pm() const { return _pm; }
	RaDec&	pm() { return _pm; }

	RaDec	position(const double epoch) const;
};

class Star;
typedef std::shared_ptr<Star>	StarPtr;

/**
 * \brief Star base class
 *
 * Stars are celestial objects that in addition have a name
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

/*
 * \brief Outline of a DeepSkyObject
 */
class Outline : public std::list<astro::RaDec> {
	std::string	_name;
public:
	const std::string&	name() const { return _name; }
	void	name(const std::string& name) { _name = name; }
	Outline(const std::string& name) : _name(name) { }
	Outline(const std::string& name, const astro::RaDec& center,
		const astro::TwoAngles& axes,
		const astro::Angle& position_angle);
	std::string	toString() const;
};

typedef std::shared_ptr<Outline>	OutlinePtr;
typedef std::list<OutlinePtr>	OutlineList;
typedef std::shared_ptr<OutlineList>	OutlineListPtr;

class MilkyWay;
typedef std::shared_ptr<MilkyWay>	MilkyWayPtr;

/**
 * \brief class to parse the 
 */
class MilkyWay : public std::map<int, OutlineListPtr> {
	static std::string	default_path;
	void	parse(std::istream& in);
public:
	typedef enum { L1, L2, L3, L4, L5 } level_t;
	MilkyWay(const std::string& filename);
	MilkyWay(std::istream& in);
	MilkyWay();
	static MilkyWayPtr	get();
	static MilkyWayPtr	get(const std::string& filename);
};

/**
 * \brief Deep Sky objects
 */
class DeepSkyObject : public CelestialObject {
	bool		_has_dimensions;
	TwoAngles	_axes;
	Angle		_position_angle;
public:
	DeepSkyObject() { _mag = 0; _has_dimensions = false; }
	virtual ~DeepSkyObject() { }
	int	number;
	std::string	name;
	std::string	constellation;
	typedef enum { Galaxy, OpenCluster, GlobularCluster, BrightNebula,
			PlanetaryNebula, ClusterNebulosity, Asterism,
			Knot, TripleStar, DoubleStar, SingleStar, Uncertain,
			Unidentified, Nonexistent, PlateDefect,
			MultipleSystem, GalaxyInMultipleSystem } object_class;
	object_class	classification;
	static std::string	classification2string(object_class);
	static object_class	string2classification(const std::string&);
	const TwoAngles&	axes() const { return _axes; }
	void	axes(const TwoAngles& a) {
		_has_dimensions = true;
		_axes = a;
	}
	const Angle&	position_angle() const { return _position_angle; }
	void	position_angle(const Angle& pa) { _position_angle = pa; }
	std::string	toString() const;
private:
	std::list<std::string>	_names;
public:
	const std::list<std::string>&	names() const { return _names; }
	void	addname(const std::string& n) { _names.push_back(n); }
	Outline	outline() const;
};

typedef std::set<DeepSkyObject> DeepSkyObjectSet;
typedef std::shared_ptr<DeepSkyObjectSet>	DeepSkyObjectSetPtr;


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

	// get a set of stars matching a name
	virtual starsetptr	findLike(const std::string& name,
					size_t maxstars = 100);
	static std::set<std::string>	starlist(const starsetptr stars);

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

	// interface for retrieving tiles
	virtual StarTilePtr	findTile(const SkyWindow& window,
				const MagnitudeRange& magrange);
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
		SAO = 1,
		Hipparcos = 2,
		Tycho2 = 3,
		Ucac4 = 4,
		Combined = 5,
		Database = 6
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

	virtual DeepSkyObjectSetPtr	find(const SkyWindow&) = 0;
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
	typedef enum deepskycatalog_e { Messier, NGCIC, PGC } deepskycatalog_t;
private:
	static std::map<deepskycatalog_t, DeepSkyCatalogPtr>	catalogmap;
public:
	DeepSkyCatalogFactory(const std::string& basedir) : _basedir(basedir) { }
	DeepSkyCatalogFactory();
	DeepSkyCatalogPtr	get(deepskycatalog_t ct);
};

/**
 * \brief Build a catalog of outlines
 */
class OutlineCatalog {
	std::map<std::string, Outline>	_outlinemap;
	void	parseOutlines(const std::string& directory);
	void	parseEllipses(const std::string& directory);
	void	parse(const std::string& directory);
public:
	OutlineCatalog();
	OutlineCatalog(const std::string& directory);
	bool	has(const std::string& name) const;
	Outline	find(const std::string& name) const;
	size_t	size() const { return _outlinemap.size(); }
};

typedef std::shared_ptr<OutlineCatalog>	OutlineCatalogPtr;

/**
 * \brief Edge of a constellation
 */
class ConstellationEdge {
	RaDec	_from;
	RaDec	_to;
public:
	ConstellationEdge(const RaDec& from, const RaDec& to)
		: _from(from), _to(to) {
	}
	const RaDec&	from() const { return _from; }
	const RaDec&	to() const { return _to; }
	bool	operator==(const ConstellationEdge& other) const;
	bool	operator<(const ConstellationEdge& other) const;
};

/**
 * \brief a single constellation
 */
class Constellation : public std::set<ConstellationEdge> {
	std::string	_name;
public:
	Constellation(const std::string& name) : _name(name) { }
	const std::string&	name() const { return _name; }
	astro::RaDec	centroid() const;
};

typedef std::shared_ptr<Constellation>	ConstellationPtr;

class ConstellationCatalog;
typedef std::shared_ptr<ConstellationCatalog>	ConstellationCatalogPtr;

/**
 * \brief Constellation Catalog class
 */
class ConstellationCatalog : public std::map<std::string, ConstellationPtr> {
public:
	ConstellationCatalog();
	static ConstellationCatalogPtr	get();
};

} // namespace catalog
} // namespace astro

#endif /* _AstroCatalog_h */
