/*
 * Backend.cpp -- database backend implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroPersistence.h>
#include <AstroFormat.h>
#include <sqlite3.h>
#include <stdexcept>
#include <AstroDebug.h>
#include <includes.h>

namespace astro {
namespace persistence {

class Sqlite3Backend;
/**
 * \brief Exception handling in Sqlite3 database
 */
class Sqlite3Exception : public std::runtime_error {
	std::string	cause(Sqlite3Backend& database,
				const std::string& info);
public:
	Sqlite3Exception(Sqlite3Backend& backend, const std::string& info) 
		: std::runtime_error(cause(backend, info)) {
	}
};

/**
 * \brief Statement abstraction for Sqlite3 database
 */
class Sqlite3Statement : public Statement {
	Sqlite3Backend&	_backend;
	sqlite3_stmt	*stmt;
public:
	Sqlite3Statement(Sqlite3Backend& backend, const std::string& query);
	~Sqlite3Statement();
	// bind parameters
        virtual void	bindInteger(int colno, int value);
        virtual void	bindDouble(int colno, double value);
        virtual void	bindString(int colno, const std::string& value);
	// executen
        virtual void	execute();
protected:
	virtual Field	field(int colno);
        virtual Row	row();
public:
        virtual Result  result();
	// retrieve values
	virtual int	integerColumn(int colno);
	virtual double	doubleColumn(int colno);
	virtual std::string	stringColumn(int colno);
};

/**
 * \brief Sqlite3 backend abstraction
 */
class Sqlite3Backend : public DatabaseBackend {
	std::string	_filename;
	sqlite3	*_database;
public:
	sqlite3	*database() { return _database; }

public:
	Sqlite3Backend(const std::string& filename);
	~Sqlite3Backend();
	virtual std::string	escape(const std::string& value);
	virtual Result	query(const std::string& query);
	virtual std::vector<std::string>
		fieldnames(const std::string& tablename);
	virtual void	begin();
	virtual void	begin(const std::string& savepoint);
	virtual void	commit();
	virtual void	commit(const std::string& savepoint);
	virtual void	rollback();
	virtual void	rollback(const std::string& savepoint);
	virtual StatementPtr	statement(const std::string& query);
	virtual bool	hastable(const std::string& tablename);
};

//////////////////////////////////////////////////////////////////////
// Sqlite 3 statement implementation
//////////////////////////////////////////////////////////////////////
/**
 * \brief Create an SQL statement
 */
Sqlite3Statement::Sqlite3Statement(Sqlite3Backend& backend,
		const std::string& query) 
	: Statement(query), _backend(backend) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "preparing statement with SQL: '%s'",
		query.c_str());
	stmt = NULL;
	int	rc;
	const char	*tail;
	if (SQLITE_OK == (rc = sqlite3_prepare_v2(_backend.database(),
		query.c_str(), query.size(), &stmt, &tail))) {
		if (NULL == stmt) {
			std::string	cause
				= stringprintf("not an sql query: '%s'",
					query.c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
			throw BadQuery(cause);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "statement prepared");
		return;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "prepare failed (%d): remaining query: %s",
		rc, tail);
	throw Sqlite3Exception(_backend,
		stringprintf("remaining query: %s", tail));
}

/**
 * \brief Destroy the statement
 */
Sqlite3Statement::~Sqlite3Statement() {
//	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy statement '%s'",
//		query().c_str());
	int	rc = sqlite3_finalize(stmt);
	if (rc != SQLITE_OK) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "error in finalize: %s",
			sqlite3_errmsg(_backend.database()));
	}
}

void	Sqlite3Statement::bindInteger(int colno, int value) {
	int	rc = SQLITE_OK;
	if (SQLITE_OK == (rc = sqlite3_bind_int(stmt, colno + 1, value))) {
		return;
	}
	throw Sqlite3Exception(_backend, "bindInteger");
}

void	Sqlite3Statement::bindDouble(int colno, double value) {
	int	rc = SQLITE_OK;
	if (SQLITE_OK == (rc = sqlite3_bind_double(stmt, colno + 1, value))) {
		return;
	}
	throw Sqlite3Exception(_backend, "bindDouble");
}

void	Sqlite3Statement::bindString(int colno, const std::string& value) {
	int	rc = SQLITE_OK;
	if (SQLITE_OK == (rc = sqlite3_bind_text(stmt, colno + 1, value.c_str(),
			value.size(), SQLITE_TRANSIENT))) {
		return;
	}
	throw Sqlite3Exception(_backend, "bindString");
}

/**
 * \brief retrieve a integer valued column
 */
int	Sqlite3Statement::integerColumn(int colno) {
	return sqlite3_column_int(stmt, colno);
}

/**
 * \brief retrieve a double valued column
 */
double	Sqlite3Statement::doubleColumn(int colno) {
	return sqlite3_column_double(stmt, colno);
}

/**
 * \brief retrieve a string valued column
 */
std::string	Sqlite3Statement::stringColumn(int colno) {
	return std::string((const char *)sqlite3_column_text(stmt, colno));
}

/**
 * \brief Execute a statement
 */
void	Sqlite3Statement::execute() {
	int	retry = 0;
	int	rc;
	while (retry < 10) {
		rc = sqlite3_step(stmt);
		switch (rc) {
		case SQLITE_OK:
		case SQLITE_DONE:
			return;
		case SQLITE_BUSY:
			retry++;
			usleep(10000);
			break;
		default:
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"sqlite3_step return code: %d", rc);
			throw Sqlite3Exception(_backend, "execute query");
		}
	}
	// step failed after 10 retries
	throw Sqlite3Exception(_backend, "execute query: after 10 retries");
}

Field	Sqlite3Statement::field(int colno) {
	std::string	name(sqlite3_column_name(stmt, colno));
	// get the value for this column
	FieldValueFactory	factory;
	FieldValuePtr	value;
	switch (sqlite3_column_type(stmt, colno)) {
	case SQLITE_INTEGER:
		value = factory.get(
			sqlite3_column_int(stmt, colno));
		break;
	case SQLITE_FLOAT:
		value = factory.get(
			sqlite3_column_double(stmt, colno));
		break;
	case SQLITE_TEXT:
		value = factory.get(std::string((const char *)
			sqlite3_column_text(stmt, colno)));
		break;
	default:
		debug(LOG_ERR, DEBUG_LOG, 0,
			"don't know how to handle this type");
		break;
	}
	return Field(name, value);
}

Row	Sqlite3Statement::row() {
	int	columns = sqlite3_column_count(stmt);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "number of columns: %d", columns);
	// process next row
	Row	newrow;
	for (int colno = 0; colno < columns; colno++) {
		newrow.push_back(field(colno));
	}
	return newrow;
}

Result	Sqlite3Statement::result() {
//	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieveing query result");
	Result	result;
	while (SQLITE_ROW == sqlite3_step(stmt)) {
//		debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieveing row");
		// process next row
		result.push_back(row());
	}
//	debug(LOG_DEBUG, DEBUG_LOG, 0, "query result has %d rows",
//		result.size());
	return result;
}

//////////////////////////////////////////////////////////////////////
// Sqlite3 Backend implementation
//////////////////////////////////////////////////////////////////////
Sqlite3Backend::Sqlite3Backend(const std::string& filename)
	: _filename(filename) {
	// check whether this version of sqlite is compiled with mutexes
	if (sqlite3_threadsafe()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "backend is thread safe");
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "backend is NOT thread safe");
	}
	// open the database
	_database = NULL;
	if (sqlite3_open(_filename.c_str(), &_database)) {
		std::string	cause
			= stringprintf("cannot open/create db on file '%s': %s",
				filename.c_str(), sqlite3_errcode(_database));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw BadDatabase(cause);
	}

	// wait 10 seconds on locked databases
	sqlite3_busy_timeout(_database, 10000);

	// some pragmas
	char	*errmesg = NULL;
	if (sqlite3_exec(_database, "PRAGMA temp_store = MEMORY;", NULL, NULL,
		&errmesg)) {
		std::string	msg = stringprintf("'PRAGMA temp_store = "
			"MEMORY' failed: %s", errmesg);
		sqlite3_free(errmesg);
		throw BadDatabase(msg);
	}
	if (sqlite3_exec(_database, "PRAGMA foreign_keys = ON;", NULL, NULL,
		&errmesg)) {
		std::string	msg = stringprintf("'PRAGMA foreign_keys = "
			"ON' failed: %s", errmesg);
		sqlite3_free(errmesg);
		throw BadDatabase(msg);
	}
	if (sqlite3_exec(_database, "PRAGMA locking_mode = NORMAL;", NULL, NULL,
		&errmesg)) {
		std::string	msg = stringprintf("'PRAGMA locking_mode = "
			"NORMAL' failed: %s", errmesg);
		sqlite3_free(errmesg);
		throw BadDatabase(msg);
	}
}

Sqlite3Backend::~Sqlite3Backend() {
	sqlite3_close(_database);
	_database = NULL;
}

std::string	Sqlite3Backend::escape(const std::string& value) {
	return value;
}

/**
 * \brief Auxiliary class to collect field values from an execute statement
 */
class ResultCollector {
	FieldValueFactory	factory;
public:	
	Result	result;
	int	operator()(int columns, char **values, char **colnames);
};

int	ResultCollector::operator()(int columns, char **values,
		char **colnames) {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "add another row with %d columns",
	//	columns);
	Row	row;
	for (int i = 0; i < columns; i++) {
		std::string	colname;
		if (colnames[i]) {
			colname = std::string(colnames[i]);
		} else {
			colname = stringprintf("col%d", i);
		}
		FieldValuePtr	colvalue = factory.get(values[i]);
		row.push_back(Field(colname, colvalue));
	}
	result.push_back(row);
	return 0;
}

static int	collector_callback(void *coll, int columns, char **values,
		char **colnames) {
	ResultCollector	*collector = (ResultCollector *)coll;
	return (*collector)(columns, values, colnames);
}

/**
 * \brief Perform an arbitrary query
 */
Result	Sqlite3Backend::query(const std::string& query) {
//	debug(LOG_DEBUG, DEBUG_LOG, 0, "query: %s", query.c_str());
	ResultCollector	collector;
	char	*errmsg = NULL;
	int	rc = sqlite3_exec(_database, query.c_str(),
			collector_callback, &collector, &errmsg);
	if (SQLITE_OK == rc) {
		return collector.result;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "query '%s' fails: %s",
		query.c_str(), errmsg);
	throw Sqlite3Exception(*this, std::string(errmsg));
}

/**
 * \brief Retreive a list of field names of a table
 *
 * The id field is always ignored.
 */
std::vector<std::string>	Sqlite3Backend::fieldnames(
					const std::string& tablename) {
	std::vector<std::string>	result;
	Result	tableinfo = query("PRAGMA table_info(" + tablename + ")");
	Result::const_iterator	rowp;
	for (rowp = tableinfo.begin(); rowp != tableinfo.end(); rowp++) {
		std::string	fieldname = (*rowp)[1]->stringValue();
		if ("id" != fieldname) {
			result.push_back(fieldname);
		}
	}
	return result;
}

/**
 * \brief Start a transaction
 */
void	Sqlite3Backend::begin() {
	query("BEGIN TRANSACTION;");
}

void	Sqlite3Backend::begin(const std::string& savepoint) {
	query("SAVEPOINT " + savepoint + ";");
}

/**
 * \brief Commit a transaction
 */
void	Sqlite3Backend::commit() {
	query("COMMIT TRANSACTION;");
}

void	Sqlite3Backend::commit(const std::string& savepoint) {
	query("RELEASE SAVEPOINT " + savepoint + ";");
}

/**
 * \brief Roll back a transaction
 */
void	Sqlite3Backend::rollback() {
	query("ROLLBACK TRANSACTION;");
}

void	Sqlite3Backend::rollback(const std::string& savepoint) {
	query("ROLLBACK TO SAVEPOINT " + savepoint + ";");
}

/**
 * \brief Create a statement from a query
 */
StatementPtr	Sqlite3Backend::statement(const std::string& query) {
	return StatementPtr(new Sqlite3Statement(*this, query));
}

/**
 * \brief Find out whether the table exists in the database
 */
bool	Sqlite3Backend::hastable(const std::string& tablename) {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "check for table %s", tablename.c_str());
	try {
		Result	res = query("PRAGMA table_info('" + tablename + "');");
		if (res.size() > 0) {
			//debug(LOG_DEBUG, DEBUG_LOG, 0, "table exists");
			return true;
		}
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot create table %s",
			x.what());
			
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
// Sqlite3Exception
//////////////////////////////////////////////////////////////////////
// this method can only be defined here because only now we know that the
// backend really is
/**
 * \brief Create an Sqlite3 error message
 */
std::string	Sqlite3Exception::cause(Sqlite3Backend& database,
			const std::string& info) {
	return stringprintf("%s: %s", info.c_str(),
		sqlite3_errmsg(database.database()));
}

//////////////////////////////////////////////////////////////////////
// BackendFactory
//////////////////////////////////////////////////////////////////////
/**
 * \brief Backend factory implementation
 */
Database	DatabaseFactory::get(const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create backend on file '%s'",
		filename.c_str());
	return Database(new Sqlite3Backend(filename));
}

} // namespace persistence
} // namespace astro
