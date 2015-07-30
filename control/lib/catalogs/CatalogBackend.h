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
	virtual void	add(int id, const Star& star);
};

/**
 * \brief Backend combining existing catalogs
 */
class FileBackend : public Catalog {
	std::string	_basedir;
	CatalogPtr	bsc_catalog;
	CatalogPtr	hipparcos_catalog;
	CatalogPtr	tycho2_catalog;
	CatalogPtr	ucac4_catalog;
public:
	FileBackend(const std::string& basedir);
	virtual ~FileBackend();
	virtual Catalog::starsetptr	find(const SkyWindow& window,
						const MagnitudeRange& magrange);
	virtual Star	find(const std::string& name);
	virtual unsigned long	numberOfStars();
};

/**
 * \brief Catalog of stars in a database
 */
class DatabaseBackend : public Catalog {
	sqlite3	*db;
	sqlite3_stmt	*stmt;
public:
	DatabaseBackend(const std::string& dbfilename);
	virtual ~DatabaseBackend();
	virtual Catalog::starsetptr	find(const SkyWindow& window,
						const MagnitudeRange& magrange);
	void	prepare();
	void	add(int id, const Star& star);
	void	finalize();
	void	clear();
	virtual Star	find(const std::string& name);
	void	createindex();
	virtual  unsigned long	numberOfStars();
};

} // namespace catalog
} // namespace astro

#endif /* _CatalogBackend_h */
