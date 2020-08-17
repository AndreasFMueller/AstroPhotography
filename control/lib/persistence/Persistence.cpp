/*
 * Persistence.cpp -- basic persistency classes implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroPersistence.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
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
// TimevalField implementation
//////////////////////////////////////////////////////////////////////
struct timeval	TimevalField::string2timeval(const std::string& s) {
	double	d = std::stod(s);
	struct timeval	t;
	t.tv_sec = floor(d);
	t.tv_usec = floor(1000000 * (d - t.tv_sec));
	return t;
}

std::string	TimevalField::timeval2string(const struct timeval& t) {
	std::string	s = TimeField::time2string(t.tv_sec);
	return stringprintf("%d.%06d", t.tv_sec, t.tv_usec);
}

TimevalField::TimevalField(const std::string& value) {
	_value = string2timeval(value);
}

TimevalField::TimevalField(double value) {
	_value.tv_sec = floor(value);
	_value.tv_usec = floor(1000000 * (value - _value.tv_sec));
}

std::string	TimevalField::stringValue() const {
	return timeval2string(_value);
}

double	TimevalField::doubleValue() const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "time value: %d.%08d", _value.tv_sec,
		_value.tv_usec);
	return _value.tv_sec + 0.000001 * _value.tv_usec;
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


FieldValuePtr	FieldValueFactory::getTimeval(const struct timeval& t) const {
	return FieldValuePtr(new TimevalField(t));
}

FieldValuePtr	FieldValueFactory::getTimeval(const std::string& value) const {
	return FieldValuePtr(new TimevalField(value));
}

FieldValuePtr	FieldValueFactory::getTimeval(double value) const {
	return FieldValuePtr(new TimevalField(value));
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
	TimevalField	*tv = dynamic_cast<TimevalField *>(&*value);
	if (tv) {
		this->bind(colno, value->doubleValue());
		//debug(LOG_DEBUG, DEBUG_LOG, 0,
		//	"bound time value '%s' to column %d",
		//	value->stringValue().c_str(), colno);
		return;
	}
	std::string	msg = stringprintf("type %s of value unknown, "
		"cannot bind", demangle_cstr(&*value));
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
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

} // namespace persistence
} // namespace astro
