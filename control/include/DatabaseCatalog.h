/*
 * DatabaseCatalog.h -- compiled catalog in a database file
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DatabaseCatalog_h
#define _DatabaseCatalog_h

#include <AstroCatalog.h>
#include <sqlite3.h>

namespace astro {
namespace catalog {

/**
 * \brief Catalog of stars in a database
 */
class DatabaseCatalog {
	sqlite3	*db;
private:
	// prevent copying
	DatabaseCatalog(const DatabaseCatalog& other);
	DatabaseCatalog&	operator=(const DatabaseCatalog& other);
public:
	DatabaseCatalog(const std::string& dbfilename);
	~DatabaseCatalog();
	Catalog::starsetptr	find(const SkyWindow& window,
					double minimum_magnitude);
	void	add(int id, const Star& star, const std::string& name);
	void	clear();
};

} // namespace catalog
} // namespace astro

#endif /* _DatabaseCatalog_h */
