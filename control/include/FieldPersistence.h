/*
 * FieldPersistence.h -- persistence of different types
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _FieldPersistence_h
#define _FieldPersistence_h

#include <AstroPersistence.h>
#include <sstream>
#include <AstroDebug.h>
#include <stdexcept>
#include <math.h>

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
	time_t	timeValue() const { return _value; }
	struct timeval	timevalValue() const {
		struct timeval	t;
		t.tv_sec = _value;
		t.tv_usec = 0;
		return t;
	}
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
	time_t	timeValue() const { return _value; }
	struct timeval	timevalValue() const {
		struct timeval	t;
		t.tv_sec = floor(_value);
		t.tv_usec = floor(1000000 * (_value - t.tv_sec));
		return t;
	}
};

//////////////////////////////////////////////////////////////////////
// fields with unix time type
//////////////////////////////////////////////////////////////////////
class TimeField : public FieldValue {
	time_t	_value;
public:
	static	time_t	string2time(const std::string& value);
	static	std::string	time2string(time_t t);
	TimeField(const std::string& value);
	TimeField(time_t t) : _value(t) { }
	std::string	stringValue() const;
	int	intValue() const { return _value; }
	double	doubleValue() const { return _value; }
	virtual std::string	toString() const {
		return "'" + stringValue() + "'";
	}
	time_t	timeValue() const { return _value; }
	struct timeval	timevalValue() const {
		struct timeval	t;
		t.tv_sec = _value;
		t.tv_usec = 0;
		return t;
	}
};

//////////////////////////////////////////////////////////////////////
// fields with unix time type
//////////////////////////////////////////////////////////////////////
class TimevalField : public FieldValue {
	struct timeval	_value;
public:
	static	struct timeval	string2timeval(const std::string& value);
	static	std::string	timeval2string(const struct timeval& t);
	TimevalField(const std::string& value);
	TimevalField(const struct timeval& t) : _value(t) { }
	TimevalField(double value);
	std::string	stringValue() const;
	int	intValue() const { return _value.tv_sec; }
	double	doubleValue() const;
	virtual std::string	toString() const {
		return "'" + stringValue() + "'";
	}
	time_t	timeValue() const { return _value.tv_sec; }
	struct timeval	timevalValue() const { return _value; }
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
	time_t	timeValue() const {
		return TimeField::string2time(_value);
	}
	struct timeval	timevalValue() const {
		return TimevalField::string2timeval(_value);
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
	virtual time_t	timeValue() const {
		throw std::runtime_error("cannot convert NULL to time_t");
	}
	virtual bool	isnull() const { return true; }
	virtual struct timeval	timevalValue() const {
		throw std::runtime_error("cannot convert NULL to struct timeval");
	}
};

} // namespace persistence
} // namespace astro

#endif /* _FieldPersistence_h */
