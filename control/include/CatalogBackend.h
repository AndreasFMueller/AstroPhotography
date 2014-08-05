/*
 * CatalogBackend.h -- compiled catalog backend
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CatalogBackend_h
#define _CatalogBackend_h

#include <AstroCatalog.h>
#include <sqlite3.h>

namespace astro {
namespace catalog {

/**
 * \brief base class for catalog backends
 */
class CatalogBackend {
private:
	// prevent copying
	CatalogBackend(const CatalogBackend& other);
	CatalogBackend&	operator=(const CatalogBackend& other);
public:
	CatalogBackend();
	~CatalogBackend();
	virtual Catalog::starsetptr	find(const SkyWindow& window,
						const MagnitudeRange& magrange);
	virtual Star	find(const std::string& name);
};

/**
 * \brief Backend combining existing catalogs
 */
class FileBackend : public CatalogBackend {
	std::string	_basedir;
	std::string	hipparcosfile;
	std::string	tycho2file;
	std::string	ucac4dir;
public:
	FileBackend(const std::string& basedir);
	~FileBackend();
	virtual Catalog::starsetptr	find(const SkyWindow& window,
						const MagnitudeRange& magrange);
	virtual Star	find(const std::string& name);
};

/**
 * \brief Catalog of stars in a database
 */
class DatabaseBackend : public CatalogBackend {
	sqlite3	*db;
public:
	DatabaseBackend(const std::string& dbfilename);
	~DatabaseBackend();
	virtual Catalog::starsetptr	find(const SkyWindow& window,
						const MagnitudeRange& magrange);
	void	add(int id, const Star& star);
	void	clear();
	virtual Star	find(const std::string& name);
};

} // namespace catalog
} // namespace astro

#endif /* _CatalogBackend_h */
