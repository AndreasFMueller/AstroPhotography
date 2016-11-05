/*
 * TableBase.cpp -- table base class implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroPersistence.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <iostream>
#include <sstream>

namespace astro {
namespace persistence {

TableBase::TableBase(Database database, const std::string& tablename,
	const std::string& createstatement)
	: _database(database), _tablename(tablename) {
	if (NULL == _database) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no database");
		throw BadDatabase("no database persent");
	}
	// test whether the database contains the table
	if (!_database->hastable(tablename)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "creating table using %s",
			createstatement.c_str());
		_database->query(createstatement);
	}

	// get all the column names
	_fieldnames = _database->fieldnames(_tablename);
}

class columnnamelist : public std::string {
	int	counter;
public:
	void	operator()(const std::string& fieldname) {
		if (counter++) {
			append(", ");
		}
		append(fieldname);
	}
};

/**
 * \brief formulate the selet query for this table
 */
std::string	TableBase::selectquery() const {
	std::ostringstream	out;
	out << "select ";
	out << std::for_each(_fieldnames.begin(), _fieldnames.end(),
		columnnamelist());
	out << " from " << _tablename << " where id = ?";
	std::string	query = out.str();
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "select query: %s", query.c_str());
	return query;
}

/**
 * \brief Find the id for the next row to be inserted
 *
 * This method generates the id 1 if there are now rows in the table
 */
long	TableBase::nextid() {
	std::ostringstream	out;
	out << "select case when count(*) = 0 then 1 else max(id + 1) end as 'nextid' from " << _tablename;
	Result	result = _database->query(out.str());
	if (result.size() != 1) {
		return 0;
	}
	Row	row = result.front();
	const FieldValuePtr&	field = row[0];
	long	id = field->intValue();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "next id: %ld", id);
	return id;
}

/**
 * \brief Find the id of the last row
 *
 * In contrast to the nextid() method, this method throws an exception if
 * there are no rows in the table
 */
long	TableBase::lastid() {
	std::ostringstream	out;
	out << "select max(id) as 'lastid' from " << _tablename;
	Result	result = _database->query(out.str());
	if (result.size() != 1) {
		std::string	cause = stringprintf("no rows in table %s",
			_tablename.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw persistence::NotFound(cause);
	}
	Row	row = result.front();
	const FieldValuePtr&	field = row[0];
	long	id = field->intValue();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "last id: %ld", id);
	return id;
}

/**
 * \brief Retrieve with a given id
 */
Row	TableBase::rowbyid(long objectid) {
	std::string	sq = selectquery(); 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "select query: %s", sq.c_str());
	StatementPtr	stmt = _database->statement(sq);
	stmt->bind(0, (int)objectid);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "object id: %d", objectid);
	Result	result = stmt->result();
	if (result.size() != 1) {
		debug(LOG_ERR, DEBUG_LOG, 0, "internal error: objectid %s",
			sq.c_str());
		throw std::runtime_error("wrong number of rows");
	}
	return result.front();
}

/**
 * \brief Add a new row, return the id
 */
long	TableBase::addrow(const UpdateSpec& updatespec) {
	int	objectid = nextid();
	std::string	query = updatespec.insertquery(_tablename);
	StatementPtr	stmt = _database->statement(query);
	updatespec.bind(stmt);
	updatespec.bindid(stmt, objectid);
	stmt->execute();
	return objectid;
}

/**
 * \brief Update a row in the database
 */
void	TableBase::updaterow(long objectid, const UpdateSpec& updatespec) {
	std::string	query = updatespec.updatequery(_tablename);
	StatementPtr	stmt = _database->statement(query);
	updatespec.bind(stmt);
	updatespec.bindid(stmt, objectid);
	stmt->execute();
}

/**
 * \brief Check whether a certain id appears in the database
 */
bool	TableBase::exists(long objectid) {
	std::ostringstream	out;
	out << "select count(*) from " << _tablename << " where id = ?";
	StatementPtr	stmt = _database->statement(out.str());
	stmt->bind(0, (int)objectid);
	Result	result = stmt->result();
	Row	row = result.front();
	return (row[0]->intValue() > 0) ? true : false;
}

/**
 * \brief Remove a row from the database
 */
void	TableBase::remove(long objectid) {
	if (!exists(objectid)) {
		return;
	}
	std::ostringstream	out;
	out << "delete from " << _tablename << " where id = ?";
	StatementPtr	stmt = _database->statement(out.str());
	stmt->bind(0, (int)objectid);
	stmt->execute();
}

/**
 * \brief Remove a list of entries
 */
void	TableBase::remove(const std::list<long>& objectids) {
	std::list<long>::const_iterator	i;
	for (i = objectids.begin(); i != objectids.end(); i++) {
		remove(*i);
	}
}

/**
 * /brief Remove all rows that match a condition
 */
void	TableBase::remove(const std::string& condition) {
	std::ostringstream	out;
	out << "delete from " << _tablename << " where " << condition;
	StatementPtr	stmt = _database->statement(out.str());
	stmt->execute();
}

/**
 * \brief retrieve a list of all object ids satisfying a condition
 */
std::list<long>	TableBase::selectids(const std::string& condition) {
	std::ostringstream	out;
	out << "select id from " << _tablename << " where " << condition;
	Result	result = _database->query(out.str());
	std::list<long>	idlist;
	Result::const_iterator	rowp;
	for (rowp = result.begin(); rowp != result.end(); rowp++) {
		idlist.push_back((*rowp)[0]->intValue());
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "list of %d ids retrieved",
	//	idlist.size());
	return idlist;
}

/**
 * \brief Retrieve with a given id
 */
Result	TableBase::selectrows(const std::string& condition) {
	std::ostringstream	out;
	out << "select id, ";
	out << std::for_each(_fieldnames.begin(), _fieldnames.end(),
		columnnamelist());
	out << " from " << _tablename;
	out << " where " << condition;
	std::string	query = out.str();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "select query: %s", query.c_str());
	StatementPtr	stmt = _database->statement(query);
	Result	result = stmt->result();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "result has %d rows", result.size());
	return result;
}

/**
 * \brief Find the record id the satisfies some uniqueness constraint
 */
long	TableBase::id(const std::string& condition) {
	std::ostringstream	out;
	out << "select id from " << _tablename << " where " << condition;
	Result	result = _database->query(out.str());
	if (1 != result.size()) {
		std::string	cause
			= stringprintf("no row for condition '%s'",
			condition.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw NotFound(cause);
	}
	return (*result.begin())[0]->intValue();
}

/**
 * \brief Find out whether there are any rows for the condition
 */
bool	TableBase::has(const std::string& condition) {
	return count(condition) > 0;
}

/**
 * \brief Count all the rows of a table
 */
long	TableBase::count() {
	return count("0 = 0");
}

/**
 * \brief Count rows that satisfy a condition
 */
long	TableBase::count(const std::string& condition) {
	std::ostringstream	out;
	out << "select count(*) from " << _tablename << " where " << condition;
	Result	result = _database->query(out.str());
	if (1 != result.size()) {
		std::string	cause
			= stringprintf("cannot count rows for condition '%s'",
				condition.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw NotFound(cause);
	}
	return (*result.begin())[0]->intValue();
}

} // namespace persistence
} // namespace astro
