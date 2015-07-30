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

	// create the table if necessary
	char	*errmsg;
	std::string	create_query(	"create table star ( "
					"    id integer not null, "
					"    ra double not null, "
					"    dec double not null, "
					"    pmra double not null, "
					"    pmdec double not null, "
					"    mag double not null, "
					"    catalog char(1) not null, "
					"    catalognumber integer not null, "
					"    primary key(id));");
	rc = sqlite3_exec(db, create_query.c_str(), NULL, NULL, &errmsg);
	if (rc != SQLITE_OK) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot create the table: %s",
			errmsg);
		sqlite3_close(db);
		throw std::runtime_error("cannot create database catalog");
	}
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

/**
 * \brief prepare the insert statement
 */
void	DatabaseBackend::prepare() {
	if (NULL != stmt) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "statement already prepared");
		return;
	}
	int	rc;
	const char	*tail;
	std::string	insert_query(
		"insert into star (id, ra, dec, pmra, pmdec, mag, catalog, "
		"                  catalognumber) "
		"values (?, ?, ?, ?, ?, ?, ?, ?);");
	if (SQLITE_OK != (rc = sqlite3_prepare_v2(db, insert_query.c_str(),
		insert_query.size(), &stmt, &tail))) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"cannot prepare insert query [%s]: %d",
			insert_query.c_str(), rc);
		stmt = NULL;
		throw std::runtime_error("cannot prepare insert");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "insert query '%s' prepared",
		insert_query.c_str());
}

/**
 * \brief Clean up the prepare statement
 */
void	DatabaseBackend::finalize() {
	if (NULL == stmt) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no statement to finalize");
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "finalizing insert statement");
	sqlite3_finalize(stmt);
	stmt = NULL;
}

/**
 * \brief add a star to the catalog
 */
void	DatabaseBackend::add(int id, const Star& star) {
	int	rc;
	bool	cleanup_needed = false;
	if (NULL == stmt) {
		prepare();
		cleanup_needed = true;
	}

	// bind the values from the star
#define	ADD_BIND_ERROR							\
	if (rc != SQLITE_OK) {						\
		std::string	msg = stringprintf("cannot bind: %d", rc);\
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());	\
		throw std::runtime_error(msg);				\
	}
	rc = sqlite3_bind_int(stmt, 1, id);
	ADD_BIND_ERROR;
	rc = sqlite3_bind_double(stmt, 2, star.ra().hours());
	ADD_BIND_ERROR;
	rc = sqlite3_bind_double(stmt, 3, star.dec().degrees());
	ADD_BIND_ERROR;
	rc = sqlite3_bind_double(stmt, 4, star.pm().ra().hours());
	ADD_BIND_ERROR;
	rc = sqlite3_bind_double(stmt, 5, star.pm().dec().degrees());
	ADD_BIND_ERROR;
	rc = sqlite3_bind_double(stmt, 6, star.mag());
	ADD_BIND_ERROR;
	char	catalog = star.catalog();
	rc = sqlite3_bind_text(stmt, 7, &catalog, 1, SQLITE_STATIC);
	ADD_BIND_ERROR;
	rc = sqlite3_bind_int(stmt, 8, star.catalognumber());
	
	rc = sqlite3_step(stmt);

	if (cleanup_needed) {
		finalize();
	} else {
		// reset needed to reuse the statement for another insert
		sqlite3_reset(stmt);
	}

	if ((rc != SQLITE_OK) && (rc != SQLITE_DONE)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot insert: %d", rc);
		throw std::runtime_error("cannot insert star");
	}
}

/**
 * \brief Clear the database
 */
void	DatabaseBackend::clear() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "clearing database");
	char	*errmsg;
	int	rc = sqlite3_exec(db, "delete from star;", NULL, NULL, &errmsg);
	if (rc != SQLITE_OK) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot clear: %s", errmsg);
		throw std::runtime_error("clear failed");
	}
}

/**
 * \brief Create an Index on RA/DEC to bring performance to an acceptable level
 */
void	DatabaseBackend::createindex() {
	// create the table if necessary
	char	*errmsg;
	std::string	query("create index staridx1 on star (dec, ra);");
	int	rc = sqlite3_exec(db, query.c_str(), NULL, NULL, &errmsg);
	if (rc != SQLITE_OK) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot create index: %s",
			errmsg);
		throw std::runtime_error("cannot create index ");
	}
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

} // namespace catalog
} // namespace astro
