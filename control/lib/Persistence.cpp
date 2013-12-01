/*
 * Persistence.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroPersistence.h>
#include <sstream>
#include <AstroDebug.h>
#include <stdexcept>

namespace astro {
namespace persistence {

//////////////////////////////////////////////////////////////////////
// fields with integer values
//////////////////////////////////////////////////////////////////////
class IntegerField : public FieldValue {
	int	_value;
public:
	IntegerField(int value) : _value(value) { }
	double	doubleValue() const { return _value; }
	int	intValue() const { return _value; }
	std::string	stringValue() const {
		std::ostringstream	out;
		out << _value;
		return out.str();
	}
};

//////////////////////////////////////////////////////////////////////
// fields with double values
//////////////////////////////////////////////////////////////////////
class DoubleField : public FieldValue {
	double	_value;
public:
	DoubleField(double value) : _value(value) { }
	double	doubleValue() const { return _value; }
	int	intValue() const { int v = _value; return v; }
	std::string	stringValue() const {
		std::ostringstream	out;
		out << _value;
		return out.str();
	}
};

//////////////////////////////////////////////////////////////////////
// fields with string values
//////////////////////////////////////////////////////////////////////
class StringField : public FieldValue {
	std::string	_value;
public:
	StringField(const std::string& value) : _value(value) { }
	std::string	stringValue() const { return _value; }
	int	intValue() const { return std::stoi(_value); }
	double	doubleValue() const { return std::stof(_value); }
	virtual std::string	toString() const {
		return "'" + stringValue() + "'";
	}
};

//////////////////////////////////////////////////////////////////////
// Null value
//////////////////////////////////////////////////////////////////////
class NullField : public FieldValue {
public:
	virtual int	intValue() const {
		throw std::runtime_error("cannot convert NULL to int");
	}
	virtual double	doubleValue() const {
		throw std::runtime_error("cannot convert NULL to double");
	}
	virtual std::string	stringValue() const {
		throw std::runtime_error("cannot convert NULL to string");
	}
	virtual bool	isnull() const { return true; }
};

//////////////////////////////////////////////////////////////////////
// FieldValueFactory implementation
//////////////////////////////////////////////////////////////////////
FieldValuePtr	FieldValueFactory::get(int value) {
	return FieldValuePtr(new IntegerField(value));
}

FieldValuePtr	FieldValueFactory::get(double value) {
	return FieldValuePtr(new DoubleField(value));
}

FieldValuePtr	FieldValueFactory::get(const std::string& value) {
	return FieldValuePtr(new StringField(value));
}

FieldValuePtr	FieldValueFactory::get(const char *value) {
	if (NULL == value) {
		return FieldValuePtr(new NullField());
	}
	return FieldValuePtr(new StringField(std::string(value)));
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
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"bound int value %d to column %d",
			value->intValue(), colno);
		return;
	}
	DoubleField	*d = dynamic_cast<DoubleField *>(&*value);
	if (d) {
		this->bind(colno, value->doubleValue());
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"bound int value %f to column %d",
			value->doubleValue(), colno);
		return;
	}
	StringField	*s = dynamic_cast<StringField *>(&*value);
	if (s) {
		this->bind(colno, value->stringValue());
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"bound int value '%s' to column %d",
			value->stringValue().c_str(), colno);
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
	void	operator()(const UpdateSpec::value_type& v) {
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
	stmt->bind(size(), id);
}

//////////////////////////////////////////////////////////////////////
// TableBase implementation
//////////////////////////////////////////////////////////////////////
TableBase::TableBase(Database& database, const std::string& tablename)
	: _database(database), _tablename(tablename) {
	// get all the rows
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "select query: %s", query.c_str());
	return query;
}

/**
 * \brief Find the id for the next row to be inserted
 */
long	TableBase::nextid() {
	std::ostringstream	out;
	out << "select max(id + 1) as 'nextid' from " << _tablename;
	Result	result = _database->query(out.str());
	if (result.size() != 1) {
		return 0;
	}
	Row	row = result.front();
	const FieldValuePtr&	field = row[0];
	return field->intValue();
}

/**
 * \brief Retrieve with a given id
 */
Row	TableBase::rowbyid(long objectid) {
	std::string	sq = selectquery(); 
	StatementPtr	stmt = _database->statement(sq);
	stmt->bind(0, (int)objectid);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "object id: %d", objectid);
	Result	result = stmt->result();
	if (result.size() != 1) {
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

} // namespace persistence
} // namespace astro
