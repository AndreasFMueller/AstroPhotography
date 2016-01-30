/*
 * Persistence.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroPersistence.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <FieldPersistence.h>
#include <sstream>
#include <AstroDebug.h>
#include <stdexcept>

namespace astro {
namespace persistence {

/**
 * \brief Parse a timestamp string from the database, convert it to Unix time
 */
time_t	TimeField::string2time(const std::string& s) {
	struct tm	t;
	t.tm_year = std::stoi(s.substr(0, 4)) - 1900;
	t.tm_mon = std::stoi(s.substr(5, 2)) - 1;
	t.tm_mday = std::stoi(s.substr(8, 2));
	t.tm_hour = std::stoi(s.substr(11, 2));
	t.tm_min = std::stoi(s.substr(14, 2));
	t.tm_sec = std::stoi(s.substr(17, 2));
        t.tm_isdst = -1;
        t.tm_zone = NULL;
        t.tm_gmtoff = 0;
	char	b[20];
	strftime(b, sizeof(b), "%Y-%m-%d %H:%M:%S", &t);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parse date: %s -> %s", s.c_str(), b);
	return mktime(&t);
}

std::string	TimeField::time2string(time_t t) {
	char	buffer[20];
	struct tm	*tmp = localtime(&t);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tmp);
	return std::string(buffer);
}

TimeField::TimeField(const std::string& value) {
	_value = string2time(value);
}

std::string	TimeField::stringValue() const {
	return time2string(_value);
}

//////////////////////////////////////////////////////////////////////
// FieldValueFactory implementation
//////////////////////////////////////////////////////////////////////
FieldValuePtr	FieldValueFactory::get(int value) const {
	return FieldValuePtr(new IntegerField(value));
}

FieldValuePtr	FieldValueFactory::get(double value) const {
	return FieldValuePtr(new DoubleField(value));
}

FieldValuePtr	FieldValueFactory::get(const std::string& value) const {
	return FieldValuePtr(new StringField(value));
}

FieldValuePtr	FieldValueFactory::get(const char *value) const {
	if (NULL == value) {
		return FieldValuePtr(new NullField());
	}
	return FieldValuePtr(new StringField(std::string(value)));
}

FieldValuePtr	FieldValueFactory::getTime(const time_t t) const {
	return FieldValuePtr(new TimeField(t));
}

FieldValuePtr	FieldValueFactory::getTime(const std::string& value) const {
	return FieldValuePtr(new TimeField(value));
}

//////////////////////////////////////////////////////////////////////
// Field methods
//////////////////////////////////////////////////////////////////////
std::ostream&	operator<<(std::ostream& out, const Field& field) {
	out << field.name() << "=" << field.value()->toString();
	return out;
}

//////////////////////////////////////////////////////////////////////
// Row methods
//////////////////////////////////////////////////////////////////////
class FieldDisplay {
	std::ostream&	_out;
	int	counter;
public:
	FieldDisplay(std::ostream& out) : _out(out) { counter = 0; }
	void	operator()(const Field& field) {
		if (counter++) {
			_out << " ";
		}
		_out << field;
	}
};

std::ostream&	operator<<(std::ostream& out, const Row& row) {
	std::for_each(row.begin(), row.end(), FieldDisplay(out));
	return out;
}

//////////////////////////////////////////////////////////////////////
// Result methods
//////////////////////////////////////////////////////////////////////
class RowDisplay {
	std::ostream&	_out;
public:
	RowDisplay(std::ostream& out) : _out(out) { }
	void	operator()(const Row& row) {
		_out << row << std::endl;
	}
};

std::ostream&	operator<<(std::ostream& out, const Result& result) {
	std::for_each(result.begin(), result.end(), RowDisplay(out));
	return out;
}

//////////////////////////////////////////////////////////////////////
// Statement methods
//////////////////////////////////////////////////////////////////////
void	Statement::bind(int colno, const FieldValuePtr& value) {
	IntegerField	*i = dynamic_cast<IntegerField *>(&*value);
	if (i) {
		this->bind(colno, value->intValue());
		//debug(LOG_DEBUG, DEBUG_LOG, 0,
		//	"bound int value %d to column %d",
		//	value->intValue(), colno);
		return;
	}
	DoubleField	*d = dynamic_cast<DoubleField *>(&*value);
	if (d) {
		this->bind(colno, value->doubleValue());
		//debug(LOG_DEBUG, DEBUG_LOG, 0,
		//	"bound double value %f to column %d",
		//	value->doubleValue(), colno);
		return;
	}
	StringField	*s = dynamic_cast<StringField *>(&*value);
	if (s) {
		this->bind(colno, value->stringValue());
		//debug(LOG_DEBUG, DEBUG_LOG, 0,
		//	"bound string value '%s' to column %d",
		//	value->stringValue().c_str(), colno);
		return;
	}
	TimeField	*t = dynamic_cast<TimeField *>(&*value);
	if (t) {
		this->bind(colno, value->stringValue());
		//debug(LOG_DEBUG, DEBUG_LOG, 0,
		//	"bound time value '%s' to column %d",
		//	value->stringValue().c_str(), colno);
		return;
	}
}

//////////////////////////////////////////////////////////////////////
// UpdateSpec implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief auxiliary class to build a comma separted list of column names
 */
class	elementlist : public std::string {
	int	counter;
public:
	void	operator()(const UpdateSpec::value_type& v) {
		if (counter++) {
			append(", ");
		}
		append(v.first);
	}
};

std::string	UpdateSpec::columnlist() const {
	return std::for_each(begin(), end(), elementlist());
}

std::string	UpdateSpec::selectquery(const std::string& tablename) const {
	std::ostringstream	out;
	out << "select " << columnlist() << " from " << tablename << " where id = ?";
	std::string	query = out.str();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "select query: %s", query.c_str());
	return query;
}

/**
 * \brief auxiliary class to build a list of question marks for values
 */
class	questionmarklist : public std::string {
	int	counter;
public:
	void	operator()(const UpdateSpec::value_type& /* v */) {
		if (counter++) {
			append(", ");
		}
		append("?");
	}
};

std::string	UpdateSpec::values() const {
	return std::for_each(begin(), end(), questionmarklist());
}

std::string	UpdateSpec::insertquery(const std::string& tablename) const {
	std::ostringstream	out;
	out << "insert into " << tablename << "(";
	out << columnlist() << ", id) values (";
	out << values() << ", ?)";
	std::string	query = out.str();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "insert query: %s", query.c_str());
	return query;
}

/**
 * \brief auxiliary class to create a set list for update statements
 */
class setlist : public std::string {
	int	counter;
public:
	void	operator()(const UpdateSpec::value_type& v) {
		if (counter++) {
			append(", ");
		}
		append(v.first);
		append(" = ?");
	}
};

std::string	UpdateSpec::update() const {
	return std::for_each(begin(), end(), setlist());
}

std::string	UpdateSpec::updatequery(const std::string& tablename) const {
	std::ostringstream	out;
	out << "update " << tablename << " set ";
	out << update();
	out << " where id = ?";
	std::string	query = out.str();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "update query: %s", query.c_str());
	return query;
}

void	UpdateSpec::bind(StatementPtr& stmt) const {
	int	index = 0;
	UpdateSpec::const_iterator	i;
	for (i = begin(); i != end(); i++, index++) {
		stmt->bind(index, i->second);
	}
}

void	UpdateSpec::bindid(StatementPtr& stmt, int id) const {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "binding id: %d", id);
	stmt->bind(size(), id);
}

//////////////////////////////////////////////////////////////////////
// TableBase implementation
//////////////////////////////////////////////////////////////////////
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
		debug(LOG_ERR, DEBUG_LOG, 0, "internal error: objectid");
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

} // namespace persistence
} // namespace astro
