/*
 * CatalogBackend.h -- compiled catalog backend
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CatalogBackend_h
#define _CatalogBackend_h

#include <AstroCatalog.h>
#include <sqlite3.h>
#include "CatalogIterator.h"

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

class FileBackendIterator;

/**
 * \brief Backend combining existing catalogs
 */
class FileBackend : public Catalog {
	std::string	_basedir;
	CatalogPtr	bsc_catalog;
	CatalogPtr	hipparcos_catalog;
	CatalogPtr	tycho2_catalog;
	CatalogPtr	ucac4_catalog;
friend class FileBackendIterator;
public:
	FileBackend(const std::string& basedir);
	virtual ~FileBackend();
	virtual Catalog::starsetptr	find(const SkyWindow& window,
						const MagnitudeRange& magrange);
	virtual Star	find(const std::string& name);
	virtual unsigned long	numberOfStars();
	virtual CatalogIterator	begin();
	virtual CatalogIterator	end();
};

/*
 * \brief Iterator class for the FileBackend
 */
class FileBackendIterator : public IteratorImplementation {
	FileBackend&	_filebackend;
	CatalogFactory::BackendType	current_backend;
	CatalogPtr	current_catalog;
	CatalogIterator	current_iterator;
public:
	FileBackendIterator(FileBackend& filebackend, bool begin_or_end);
	virtual ~FileBackendIterator();
	virtual Star	operator*();
	bool	operator==(const FileBackendIterator& other) const;
	virtual bool	operator==(const IteratorImplementation& other) const;
	virtual std::string	toString() const;
	virtual	void	increment();
};

/**
 * \brief Catalog of stars in a database
 */
class DatabaseBackend : public Catalog {
	sqlite3	*db;
public:
	DatabaseBackend(const std::string& dbfilename);
	virtual ~DatabaseBackend();
	virtual Catalog::starsetptr	find(const SkyWindow& window,
						const MagnitudeRange& magrange);
	virtual Star	find(const std::string& name);
	virtual  unsigned long	numberOfStars();
	CatalogIterator	begin();
	CatalogIterator	end();
};

/**
 * \brief Class to create a backend database
 */
class DatabaseBackendCreator {
	sqlite3	*db;
	sqlite3_stmt	*stmt;
	int	id;
	// private copy and assignment constructors
	DatabaseBackendCreator(const DatabaseBackendCreator& other);
	DatabaseBackendCreator&	operator=(const DatabaseBackendCreator& other);
public:
	DatabaseBackendCreator(const std::string& dbfilename);
	~DatabaseBackendCreator();
	void	prepare();
	void	finalize();
	void	createindex();
	void	clear();
	void	add(const Star& star);
};

/**
 * \brief Iterator for the database
 */
class DatabaseBackendIterator : public IteratorImplementation {
	sqlite3_stmt	*stmt;
	StarPtr	current_star;
	int	id;
public:
	DatabaseBackendIterator(sqlite3 *db, bool begin_or_end);
	virtual ~DatabaseBackendIterator();
	virtual Star	operator*();
	virtual bool	operator==(const IteratorImplementation& other) const;
	virtual bool	operator==(const DatabaseBackendIterator& other) const;
	virtual std::string	toString() const;
	virtual void	increment();
};

} // namespace catalog
} // namespace astro

#endif /* _CatalogBackend_h */
