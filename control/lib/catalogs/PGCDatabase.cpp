/*
 * PGCDatabase.cpp -- database backend for the PGC catalog
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <PGC.h>

namespace astro {
namespace catalog {

/**
 * \brief constructor for the PGC database backend
 *
 * \param dbfilename	filename for the database
 */
PGCDatabase::PGCDatabase(const std::string& dbfilename)
	: _dbfilename(dbfilename) {
	// open the database
	if (sqlite3_open(_dbfilename.c_str(), &_database)) {
		std::string     cause
			= stringprintf("cannot open/create db on file '%s': %s",
				dbfilename.c_str(), sqlite3_errmsg(_database));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw std::runtime_error(cause);
	}

	// find out whether the pgc table already exists
	int	rc;
        const char	*tail;
        std::string	table_query(	"select count(*) "
                                        "from sqlite_master "
                                        "where type = 'table' "
                                        "  and name = 'pgc';");
        debug(LOG_DEBUG, DEBUG_LOG, 0, "preparing query [%s]",
                table_query.c_str());
	sqlite3_stmt    *stmt;
        if (SQLITE_OK != (rc = sqlite3_prepare_v2(_database,
		table_query.c_str(), table_query.size(), &stmt, &tail))) {
                debug(LOG_DEBUG, DEBUG_LOG, 0,
                        "cannot query for pgc table: %d", rc);
                sqlite3_close(_database);
                throw std::runtime_error("cannot prepare pgc table query");
        }

        // execute the query
        if (SQLITE_ROW != (rc = sqlite3_step(stmt))) {
                sqlite3_close(_database);
                debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot execute: %d", rc);
                throw std::runtime_error("cannot execute pgc table query");
        }

        // retrieve the results
        int     count = sqlite3_column_int(stmt, 0);
        debug(LOG_DEBUG, DEBUG_LOG, 0, "number of pgc tables: %d", count);
	

	// create the pgc table
	char    *errmsg;
	std::string	create_query(	"create table pgc ( "
					"    id integer not null, "
					"    name varchar(16) not null, "
					"    pgcname char(10), "
					"    ra double not null, "
					"    dec double not null, "
					"    major double not null, "
					"    minor double not null, "
					"    pa double not null, "
					"    primary key(id));");
	rc = sqlite3_exec(_database, create_query.c_str(), NULL, NULL, &errmsg);
	if (rc != SQLITE_OK) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot create the table: %s",
			errmsg);
		sqlite3_close(_database);
		throw std::runtime_error("cannot create database catalog");
	}

	// prepare the insert query
	std::string	query(  "insert into pgc(id, name, pgcname, ra, dec, "
				"                major, minor, pa)"
				"values(?, ?, ?, ?, ?, ?, ?, ?)");
        if (SQLITE_OK != (rc = sqlite3_prepare_v2(_database, query.c_str(),
                query.size(), &_insert_stmt, &tail))) {
                debug(LOG_DEBUG, DEBUG_LOG, 0,
                        "cannot prepare insert query [%s]: %d",
                        query.c_str(), rc);
                stmt = NULL;
                throw std::runtime_error("cannot prepare insert");
        }

}

PGCDatabase::~PGCDatabase() {
	sqlite3_close(_database);
	_database = NULL;
}

/**
 * \brief add a deep sky object
 *
 * \param deepskyobject		DSO to add
 */
void	PGCDatabase::add(const DeepSkyObject& deepskyobject) {
	sqlite3_bind_int64(_insert_stmt, 1, deepskyobject.number);
	sqlite3_bind_text(_insert_stmt, 2, deepskyobject.name.c_str(),
		deepskyobject.name.size(), SQLITE_STATIC);
	sqlite3_bind_text(_insert_stmt, 3, deepskyobject.name.c_str(),
		deepskyobject.name.size(), SQLITE_STATIC);
	sqlite3_bind_double(_insert_stmt, 4,
		deepskyobject.position(2000).ra().hours());
	sqlite3_bind_double(_insert_stmt, 5,
		deepskyobject.position(2000).dec().degrees());
	sqlite3_bind_double(_insert_stmt, 6,
		deepskyobject.axes().a1().degrees());
	sqlite3_bind_double(_insert_stmt, 7,
		deepskyobject.axes().a2().degrees());
	sqlite3_bind_double(_insert_stmt, 8,
		deepskyobject.position_angle().degrees());
	sqlite3_step(_insert_stmt);
	sqlite3_stmt	*is = _insert_stmt;
	for_each(deepskyobject.names().begin(), deepskyobject.names().end(),
		[is,deepskyobject](const std::string& name) mutable {
			sqlite3_bind_int64(is, 1, deepskyobject.number);
			sqlite3_bind_text(is, 2, name.c_str(),
				name.size(), SQLITE_STATIC);
			sqlite3_bind_text(is, 3,
				deepskyobject.name.c_str(),
				deepskyobject.name.size(), SQLITE_STATIC);
			sqlite3_bind_double(is, 4,
				deepskyobject.position(2000).ra().hours());
			sqlite3_bind_double(is, 5,
				deepskyobject.position(2000).dec().degrees());
			sqlite3_bind_double(is, 6,
				deepskyobject.axes().a1().degrees());
			sqlite3_bind_double(is, 7,
				deepskyobject.axes().a2().degrees());
			sqlite3_bind_double(is, 8,
				deepskyobject.position_angle().degrees());
			sqlite3_step(is);
		}
	);
}

/**
 * \brief find a deep sky object in the database
 */
DeepSkyObject	PGCDatabase::find(const std::string& name) {
	std::string	f("select a.id, a.name, a.pgcname, a.ra, a.dec, "
			  "       a.major, a.minor, a.pa "
			  "from pgc a, pgc b "
			  "where a.id = b.id "
			  "  and b.name = ? "
			  "  and b.name = b.pgcname "
			  "order by 1 asc");
	int	rc;
	sqlite3_stmt	*stmt = NULL;
	const char	*tail = NULL;
	if (SQLITE_OK != (rc = sqlite3_prepare_v2(_database, f.c_str(),
                f.size(), &stmt, &tail))) {
                debug(LOG_DEBUG, DEBUG_LOG, 0,
                        "cannot query for pgc table: %d", rc);
                sqlite3_close(_database);
                throw std::runtime_error("cannot prepare pgc table query");
        }
	
	sqlite3_bind_text(stmt, 1, name.c_str(), name.size(), SQLITE_STATIC);
	rc = sqlite3_step(stmt);

	DeepSkyObject	result;
	result.ra() = astro::Angle(sqlite3_column_double(stmt, 3),
		Angle::Hours);
	result.dec() = astro::Angle(sqlite3_column_double(stmt, 4),
		Angle::Degrees);
	result.number = sqlite3_column_int64(stmt, 0);
	result.name = std::string((char *)sqlite3_column_text(stmt, 2));
	result.axes(astro::TwoAngles(
		astro::Angle(sqlite3_column_double(stmt, 5) / 3600.,
			Angle::Degrees),
		astro::Angle(sqlite3_column_double(stmt, 6) / 3600.,
			Angle::Degrees)));
	result.position_angle(Angle(sqlite3_column_double(stmt, 7),
		Angle::Degrees));

	// now add all the names
	rc = sqlite3_step(stmt);
	while (rc == SQLITE_OK) {
		result.addname(std::string((char *)sqlite3_column_text(stmt, 1)));
		rc = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return result;
}

/**
 * \brief Find objects within a window on the sky
 *
 * \param window	window in the sky
 */
DeepSkyObjectSetPtr	PGCDatabase::find(const SkyWindow& window) {
	std::string	f("select pgcname from pgc "
			  "where ? <= dec and dec <= ? "
			  "  and ((? <= ra and ra <= ?) "
			  "   or  (? > ? and (ra <= ? or ? <= ra)))"
			);
	int	rc;
	sqlite3_stmt	*stmt = NULL;
	const char	*tail = NULL;
	if (SQLITE_OK != (rc = sqlite3_prepare_v2(_database, f.c_str(),
                f.size(), &stmt, &tail))) {
                debug(LOG_DEBUG, DEBUG_LOG, 0,
                        "cannot query for pgc table: %d", rc);
                sqlite3_close(_database);
                throw std::runtime_error("cannot prepare pgc table query");
        }
	
	sqlite3_bind_double(stmt, 1, window.topdec().degrees());
	sqlite3_bind_double(stmt, 2, window.bottomdec().degrees());

	sqlite3_bind_double(stmt, 3, window.leftra().hours());
	sqlite3_bind_double(stmt, 4, window.rightra().hours());

	sqlite3_bind_double(stmt, 5, window.leftra().hours());
	sqlite3_bind_double(stmt, 6, window.rightra().hours());

	sqlite3_bind_double(stmt, 7, window.leftra().hours());
	sqlite3_bind_double(stmt, 8, window.rightra().hours());

	rc = sqlite3_step(stmt);
	DeepSkyObjectSet	*resultset = new DeepSkyObjectSet();
	DeepSkyObjectSetPtr	result(resultset);
	while (rc == SQLITE_OK) {
		std::string	name((char *)sqlite3_column_text(stmt, 1));
		DeepSkyObject	dso = find(name);
		if (window.contains(dso)) {
			result->insert(dso);
		}
		rc = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);

	return result;
}

/**
 * \brief retrieve objects by prefix
 *
 * \param name		name prefix
 * \param maxobjects	maximum namber of objects to return
 */
std::set<std::string>	PGCDatabase::findLike(const std::string& name,
	size_t maxobjects) {
	std::string	f("select a.pgcname "
			  "from pgc a "
			  "where a.name like ? "
			  "  and a.name = a.pgcname "
			  "order by 1 asc "
			  "limit ?");
	int	rc;
	sqlite3_stmt	*stmt = NULL;
	const char	*tail = NULL;
	if (SQLITE_OK != (rc = sqlite3_prepare_v2(_database, f.c_str(),
                f.size(), &stmt, &tail))) {
                debug(LOG_DEBUG, DEBUG_LOG, 0,
                        "cannot query for pgc table: %d", rc);
                sqlite3_close(_database);
                throw std::runtime_error("cannot prepare pgc table query");
        }
	
	std::string	prefix = name;
	if (prefix.find('%') == std::string::npos) {
		prefix = prefix + "%";
	}
	sqlite3_bind_text(stmt, 1, prefix.c_str(), prefix.size(),
		SQLITE_STATIC);
	sqlite3_bind_int64(stmt, 2, maxobjects);

	std::set<std::string>	result;
	rc = sqlite3_step(stmt);
	while (rc == SQLITE_OK) {
		result.insert(std::string((char *)sqlite3_column_text(stmt, 1)));
		rc = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return result;
}

/**
 * \brief Determine number of objects in the database
 */
size_t	PGCDatabase::size() {
	std::string	query("select count(*) from pgc");
	sqlite3_stmt	*stmt;
	const char      *tail;

	int     rc;
	if (SQLITE_OK != (rc = sqlite3_prepare_v2(_database, query.c_str(),
		query.size(), &stmt, &tail))) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"cannot query for number of stars: %d", rc);
		throw std::runtime_error("cannot prepare pgc table query");
	}

	// execute the query
	if (SQLITE_ROW != (rc = sqlite3_step(stmt))) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot execute: %d", rc);
		sqlite3_reset(stmt);
		throw std::runtime_error("cannot execute pgc table query");
	}

	// retrieve the results
	size_t     count = sqlite3_column_int(stmt, 0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "number of pgc tables: %ld", count);

	// cleanup the prepared statement
	sqlite3_finalize(stmt);

	// return the number of records in the database
	return count;
}

} // namespace catalog
} // namespace astro
