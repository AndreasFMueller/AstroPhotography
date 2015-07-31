/*
 * DatabaseBackendIterator.cpp
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochscule Rapperswil
 */
#include "CatalogBackend.h"

namespace astro {
namespace catalog {

DatabaseBackendIterator::DatabaseBackendIterator(sqlite3 *db,
	bool begin_or_end) : IteratorImplementation(begin_or_end) {
	id = 0;
	if (!begin_or_end) {
		stmt = NULL;
		return;
	}
	const char	*query =
		"select id, ra, dec, pmra, pmdec, mag, catalog, catalognumber, "
		"       name, longname "
		"from star "
		"order by id";

	// prepare the statement
	const char	*tail = NULL;
	int	rc;
	if (SQLITE_OK != (rc = sqlite3_prepare_v2(db, query, strlen(query),
		&stmt, &tail))) {
		throw std::runtime_error("cannot prepare star lookup");
	}

	increment();
}

DatabaseBackendIterator::DatabaseBackendIterator(sqlite3 *db,
	const SkyWindow& window, const MagnitudeRange& magrange)
	: IteratorImplementation(true) {
	id = 0;
	double	left = window.leftra().hours();
	double	right = window.rightra().hours();

	char	query[1024];
	strcpy(query,
		"select id, ra, dec, pmra, pmdec, mag, catalog, catalognumber, "
		"       name, longname "
		"from star "
		"where ? <= mag and mag <= ? "
		"and ? <= dec and dec <= ? ");
	if (left < right) {
		strcat(query, "and ? <= ra and ra <= ? ");
	} else {
		strcat(query, "and (ra <= ? or ? <= ra)");
	}
	strcat(query, "order by id");

	// prepare the statement
	const char	*tail = NULL;
	int	rc;
	if (SQLITE_OK != (rc = sqlite3_prepare_v2(db, query, strlen(query),
		&stmt, &tail))) {
		throw std::runtime_error("cannot prepare star lookup");
	}

	// bind the variables
	rc = sqlite3_bind_double(stmt, 1, magrange.brightest());
	ADD_BIND_ERROR;
	rc = sqlite3_bind_double(stmt, 2, magrange.faintest());
	ADD_BIND_ERROR;
	std::pair<double, double>	decs = window.decinterval();
	rc = sqlite3_bind_double(stmt, 3, decs.first);
	ADD_BIND_ERROR;
	rc = sqlite3_bind_double(stmt, 4, decs.second);
	ADD_BIND_ERROR;
	rc = sqlite3_bind_double(stmt, 5, left);
	ADD_BIND_ERROR;
	rc = sqlite3_bind_double(stmt, 6, right);
	ADD_BIND_ERROR;
	rc = sqlite3_bind_double(stmt, 7, left);
	ADD_BIND_ERROR;
	rc = sqlite3_bind_double(stmt, 8, right);
	ADD_BIND_ERROR;

	increment();
}

DatabaseBackendIterator::~DatabaseBackendIterator() {
	if (stmt) {
		sqlite3_finalize(stmt);
		stmt = NULL;
	}
}

Star	DatabaseBackendIterator::operator*() {
	return *current_star;
}

bool	DatabaseBackendIterator::operator==(const IteratorImplementation& other) const {
	return equal_implementation(this, other);
}

bool	DatabaseBackendIterator::operator==(const DatabaseBackendIterator& other) const {
	if (stmt != other.stmt) {
		return false;
	}

	// comparison of end iterators
	if (isEnd() && other.isEnd()) {
		return true;
	}

	// comparision of nontrivial iterators
	return (id == other.id);
}

std::string	DatabaseBackendIterator::toString() const {
	return stringprintf("[%d] %s", id, current_star->toString().c_str());
}

void	DatabaseBackendIterator::increment() {
	// perform step
	int	rc = sqlite3_step(stmt);
	if (rc != SQLITE_OK) {
		sqlite3_finalize(stmt);
		stmt = NULL;
		_isEnd = false;
		return;
	}

	// read the star information
	id = sqlite3_column_int(stmt, 0);
	double	ra = sqlite3_column_double(stmt, 1);;
	double	dec = sqlite3_column_double(stmt, 2);;
	double	pmra = sqlite3_column_double(stmt, 3);;
	double	pmdec = sqlite3_column_double(stmt, 4);;
	double	mag = sqlite3_column_double(stmt, 5);;
	char	catalog = sqlite3_column_text(stmt, 6)[0];
	long long	catalognumber = sqlite3_column_int(stmt, 7);
	const unsigned char	*name = sqlite3_column_text(stmt, 8);
	const unsigned char	*longname = sqlite3_column_text(stmt, 9);

	// create the current star object
	current_star = StarPtr(new Star(std::string((const char *)name)));
	current_star->ra().hours(ra);
	current_star->dec().degrees(dec);
	RaDec	pm;
	pm.ra().hours(pmra);
	pm.dec().degrees(pmdec);
	//current_star->pm(pm);
	current_star->mag(mag);
	current_star->catalog(catalog);
	current_star->catalognumber(catalognumber);
	current_star->longname(std::string((char *)longname));
}

} // namespace catalog
} // namespace astro
