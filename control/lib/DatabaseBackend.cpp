/*
 * DatabaseBackend.cpp -- Database Catalog implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CatalogBackend.h>

namespace astro {
namespace catalog {

DatabaseBackend::DatabaseBackend(const std::string& dbfilename) {
	// open the database
	if (sqlite3_open(dbfilename.c_str(), &db)) {
		throw std::runtime_error("cannot open/create database");
	}

	// prepare a query to find out whether the table already exists
	int	rc;
	sqlite3_stmt	*stmt;
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
					"    name varchar(16) not null, "
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

/**
 * \brief Retrieve stars in a window up to a given magnitude
 */
Catalog::starsetptr	DatabaseBackend::find(const SkyWindow& window,
			double minimum_magnitude) {
	std::set<Star>	*stars = new std::set<Star>;
	Catalog::starsetptr	result(stars);
	int	rc;
	sqlite3_stmt	*stmt;
	const char	*tail;
	std::string	query(	"select ra, dec, pmra, pmdec, mag "
				"from star "
				"where mag < ? "
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
	sqlite3_bind_double(stmt, 1, minimum_magnitude);
	double	ramax = (window.center().ra() + window.rawidth() * 0.5).hours();
	double	ramin = (window.center().ra() - window.rawidth() * 0.5).hours();
	sqlite3_bind_double(stmt, 2, ramin);
	sqlite3_bind_double(stmt, 3, ramax);
	double	decmax = (window.center().dec()
				+ window.decheight() * 0.5).degrees();
	double	decmin = (window.center().dec()
				- window.decheight() * 0.5).degrees();
	sqlite3_bind_double(stmt, 4, decmin);
	sqlite3_bind_double(stmt, 5, decmax);

	// execute the query
	rc = sqlite3_step(stmt);
	while (rc == SQLITE_OK) {
		Star	star;
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
		star.mag() = mag;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "adding star %s to result",
			star.toString().c_str());
		stars->insert(star);
		rc = sqlite3_step(stmt);
	}

	sqlite3_finalize(stmt);
	return result;
}

/**
 * \brief add a star to the catalog
 */
void	DatabaseBackend::add(int id, const Star& star, const std::string& name) {
	int	rc;
	sqlite3_stmt	*stmt;
	const char	*tail;
	std::string	insert_query(
		"insert into star (id, ra, dec, pmra, pmdec, mag, name) "
		"values (?, ?, ?, ?, ?, ?, ?);");
	if (SQLITE_OK != (rc = sqlite3_prepare_v2(db, insert_query.c_str(),
		insert_query.size(), &stmt, &tail))) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"cannot prepare insert query [%s]: %d",
			insert_query.c_str(), rc);
		throw std::runtime_error("cannot prepare insert");
	}

	// bind the values from the star
	sqlite3_bind_int(stmt, 1, id);
	sqlite3_bind_double(stmt, 2, star.ra().hours());
	sqlite3_bind_double(stmt, 3, star.dec().degrees());
	sqlite3_bind_double(stmt, 4, star.pm().ra().hours());
	sqlite3_bind_double(stmt, 5, star.pm().dec().degrees());
	sqlite3_bind_double(stmt, 6, star.mag());
	sqlite3_bind_text(stmt, 7, name.c_str(), name.size(),
		SQLITE_TRANSIENT);
	
	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
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

} // namespace catalog
} // namespace astro
