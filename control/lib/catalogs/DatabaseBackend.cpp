/*
 * DatabaseBackend.cpp -- Database Catalog implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "CatalogBackend.h"
#include <AstroFormat.h>
#include "Ucac4.h"

namespace astro {
namespace catalog {

DatabaseBackend::DatabaseBackend(const std::string& dbfilename) {
	// open the database
	if (sqlite3_open(dbfilename.c_str(), &db)) {
		throw std::runtime_error("cannot open/create database");
	}

	// prepare a query to find out whether the table already exists
	int	rc;
	const char	*tail;
	std::string	table_query(	"select count(*) "
					"from sqlite_master "
					"where type = 'table' "
					"  and name = 'star';");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "preparing query [%s]",
		table_query.c_str());
	sqlite3_stmt	*stmt;
	if (SQLITE_OK != (rc = sqlite3_prepare_v2(db, table_query.c_str(),
		table_query.size(), &stmt, &tail))) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot query for star table: %d", rc);
		sqlite3_close(db);
		throw std::runtime_error("cannot prepare star table query");
	}

	// execute the query
	if (SQLITE_ROW != (rc = sqlite3_step(stmt))) {
		sqlite3_close(db);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot execute: %d", rc);
		throw std::runtime_error("cannot execute star table query");
	}
	
	// retrieve the results
	int	count = sqlite3_column_int(stmt, 0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "number of star tables: %d", count);

	// cleanup
	sqlite3_finalize(stmt);
	stmt = NULL;

	// check whether table exists
	if (count == 1) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "star table already exists");
		return;
	}

	sqlite3_close(db);
	throw std::runtime_error("star table does not exist");
}

/**
 * \brief close the database
 */
DatabaseBackend::~DatabaseBackend() {
	sqlite3_close(db);
}

static std::string	starname(char catalog, uint32_t catalognumber) {
	switch (catalog) {
	case 'H':
		return stringprintf("HIP%u", catalognumber);
	case 'T':
		return stringprintf("T%u", catalognumber);
	case 'U':
		return stringprintf("UCAC4-%u-%u",
			catalognumber / 1000000,
			catalognumber % 1000000);
	}
	throw std::runtime_error("unkonwn catalog");
}

/**
 * \brief Retrieve stars in a window up to a given magnitude
 */
Catalog::starsetptr	DatabaseBackend::find(const SkyWindow& window,
				const MagnitudeRange& magrange) {
	std::set<Star>	*stars = new std::set<Star>;
	Catalog::starsetptr	result(stars);
	int	rc;
	sqlite3_stmt	*stmt;
	const char	*tail;
	std::string	query(	"select ra, dec, pmra, pmdec, mag, catalog, "
				"       catalognumber "
				"from star "
				"where mag <= ? and mag >= ? "
				"  and ? <= ra and ra <= ? "
				"  and ? <= dec and dec <= ?");
	if (SQLITE_OK != (rc = sqlite3_prepare_v2(db, query.c_str(),
		query.size(), &stmt, &tail))) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"cannot prepare select query [%s]: %d",
			query.c_str(), rc);
		throw std::runtime_error("cannot prepare select");
	}

	// bind the values
	sqlite3_bind_double(stmt, 1, magrange.faintest());
	sqlite3_bind_double(stmt, 2, magrange.brightest());
	double	ramax = (window.center().ra() + window.rawidth() * 0.5).hours();
	double	ramin = (window.center().ra() - window.rawidth() * 0.5).hours();
	sqlite3_bind_double(stmt, 3, ramin);
	sqlite3_bind_double(stmt, 4, ramax);
	double	decmax = (window.center().dec()
				+ window.decheight() * 0.5).degrees();
	double	decmin = (window.center().dec()
				- window.decheight() * 0.5).degrees();
	sqlite3_bind_double(stmt, 5, decmin);
	sqlite3_bind_double(stmt, 6, decmax);

	// execute the query
	rc = sqlite3_step(stmt);
	while (rc == SQLITE_OK) {
		char	catalog = sqlite3_column_text(stmt, 5)[0];
		uint32_t	catalognumber = sqlite3_column_int(stmt, 6);
		Star	star(starname(catalog, catalognumber));
		double	ra = sqlite3_column_double(stmt, 0);
		star.ra().hours(ra);
		double	dec = sqlite3_column_double(stmt, 1);
		star.dec().degrees(dec);
		RaDec	pm;
		double	pmra = sqlite3_column_double(stmt, 2);
		star.pm().ra().hours(pmra);
		double	pmdec = sqlite3_column_double(stmt, 3);
		star.pm().dec().degrees(pmdec);
		double	mag = sqlite3_column_double(stmt, 4);
		star.mag(mag);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "adding star %s to result",
			star.toString().c_str());
		stars->insert(star);
		rc = sqlite3_step(stmt);
	}

	sqlite3_finalize(stmt);
	return result;
}

Star	DatabaseBackend::find(const std::string& name) {
	int	rc;
	sqlite3_stmt	*stmt;
	const char	*tail;

	char	catalog = name[0];
	uint32_t	catalognumber;
	switch (catalog) {
	case 'H':	catalognumber = std::stoi(name.substr(3)); break;
	case 'T':	catalognumber = std::stoi(name.substr(1)); break;
	case 'U':	catalognumber = Ucac4StarNumber(name).catalognumber();
			break;
	}

	std::string	query(	"select ra, dec, pmra, pmdec, mag "
				"from star where catalog = ? "
				" and catalognumber = ?");
	if (SQLITE_OK != (rc = sqlite3_prepare_v2(db, query.c_str(),
		query.size(), &stmt, &tail))) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"cannot prepare select query [%s]: %d",
			query.c_str(), rc);
		throw std::runtime_error("cannot prepare select");
	}

	// bind the values
	sqlite3_bind_text(stmt, 1, &catalog, 1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, catalognumber);

	// execute the query
	rc = sqlite3_step(stmt);

	Star	star(starname(catalog, catalognumber));

	double	ra = sqlite3_column_double(stmt, 0);
	star.ra().hours(ra);
	double	dec = sqlite3_column_double(stmt, 1);
	star.dec().degrees(dec);
	RaDec	pm;
	double	pmra = sqlite3_column_double(stmt, 2);
	star.pm().ra().hours(pmra);
	double	pmdec = sqlite3_column_double(stmt, 3);
	star.pm().dec().degrees(pmdec);
	double	mag = sqlite3_column_double(stmt, 4);
	star.mag(mag);

	sqlite3_finalize(stmt);
	return star;
}

unsigned long	DatabaseBackend::numberOfStars() {
	std::string	query("select count(*) from star");
	sqlite3_stmt	*stmt2;
	const char	*tail;

	int	rc;
	if (SQLITE_OK != (rc = sqlite3_prepare_v2(db, query.c_str(),
                query.size(), &stmt2, &tail))) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"cannot query for number of stars: %d", rc);
		sqlite3_close(db);
		throw std::runtime_error("cannot prepare star table query");
        }

        // execute the query
	if (SQLITE_ROW != (rc = sqlite3_step(stmt2))) {
		sqlite3_close(db);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot execute: %d", rc);
		throw std::runtime_error("cannot execute star table query");
	}

        // retrieve the results
	int	count = sqlite3_column_int(stmt2, 0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "number of star tables: %d", count);

	// cleanup the prepared statement
	sqlite3_finalize(stmt2);

	// return the number of records in the database
	return count;
}

CatalogIterator	DatabaseBackend::begin() {
	IteratorImplementationPtr	impl;
	return CatalogIterator(impl);
}

CatalogIterator DatabaseBackend::end() {
	IteratorImplementationPtr	impl;
	return CatalogIterator(impl);
}

} // namespace catalog
} // namespace astro
