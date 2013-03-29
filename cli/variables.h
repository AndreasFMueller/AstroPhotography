/*
 * variables.h -- variable handling for the astro command language interpreter
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _variables_h
#define _variables_h

#include <string>
#include <map>
#include <tr1/memory>
#include <ostream>

namespace astro {
namespace cli {

/**
 * \brief Base class for variables
 */
class value_base {
public:
	value_base() { }
	virtual ~value_base() { }
};

/**
 * \brief Template class for variables, takes care of the payload data types
 */
template<typename T>
class value : public value_base {
	/* shared pointer for the payload */
	std::tr1::shared_ptr<T>	v;
public:
	// constructors/destructors
	value<T>(T *payload) : v(payload) { }

	virtual ~value<T>() { }

	// return the value of the variable, making a copy if necessary
	T*	val() {
		return &*v;
	}
	std::tr1::shared_ptr<T>	valptr() {
		return v;
	}

	// assign variables
	value<T>&	operator=(const value<T>& other) {
		v = other.v;
		return *this;
	}
};

template<typename T>
std::ostream&	operator<<(std::ostream& out, const value<T>& var) {
	return out << *var.value();
}


/**
 * \brief Variable type
 *
 * Variables are handled through a shared pointer that owns the variable
 * payload. This means that as soon as a variable is in some map, the
 * data wont be lost. The downside is that as soon as the table is 
 * destroyed, the payload data is also destroyed.
 */
typedef	std::tr1::shared_ptr<value_base>	ValuePtr;

std::ostream&	operator<<(std::ostream& out, const ValuePtr varp);

/**
 * \brief A symbol table
 */
class	variables : public std::map<std::string, ValuePtr> {
public:
	friend std::ostream&	operator<<(std::ostream& out,
		const variables& table);
	bool	contains(const std::string& name) const;
};

std::ostream&	operator<<(std::ostream& out, const variables& table);

} // namespace cli
} // namespace astrocli

#endif /* _variables_h */
