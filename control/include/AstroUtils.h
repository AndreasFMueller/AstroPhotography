/*
 * AstroUtils.h -- some utility classes
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroUtils_h
#define _AstroUtils_h

#include <string>
#include <vector>
#include <set>
#include <map>
#include <AstroDebug.h>
#include <pthread.h>

namespace astro {

/**
 * \brief Timer class
 *
 * Some processes, in particular the SX driver, need to know exactly how long
 * a given process takes. In the SX driver for the M26C camera this is used
 * to correct the two fields for exposure differences.
 */
class Timer {
	double	_startTime;
public:
	const double&	startTime() const { return _startTime; }
private:
	double	_endTime;
public:
	const double&	endTime() const { return _endTime; }
public:
	static double	gettime();
	static void	sleep(double t);
	Timer();
	void	start();
	void	end();
	double	elapsed();
};

/**
 * \brief Concatenator functor class
 *
 * quite often, a vector or set of strings need to be concatenated to a
 * single string, e.g. for display or to build a type of URL. This functor
 * class can be used with the for_each algorithm to accomplish this.
 */
class Concatenator {
	std::string	_separator;
public:
	const std::string&	separator() const { return _separator; }
	void	separator(const std::string& s) { _separator = s; }
private:
	std::string	_result;
public:
	const std::string&	result() const { return _result; }
private:
	unsigned int	_componentcount;
public:
	unsigned int	componentcount() const { return _componentcount; }
public:
	Concatenator(const std::string& separator)
		: _separator(separator), _componentcount(0) { }
	void	operator()(const std::string&);
	operator std::string() const { return _result; }
	
	static	std::string	concat(const std::vector<std::string>& data,
					const std::string& separator);
	static	std::string	concat(const std::set<std::string>& data,
					const std::string& separator);
};

/**
 * \brief Splitter algorithm
 *
 * Splitting is an often used operation in parsing names, here we provide
 * a template function that works nicely for all kinds of containers.
 */

template<typename container>
container&	split(const std::string& data, const std::string& separator,
			container& cont) {
	std::string	work = data;
	std::string::size_type	pos;
	while (std::string::npos != (pos = work.find(separator))) {
		std::string	component = work.substr(0, pos);
		cont.insert(cont.end(), component);
		work = work.substr(pos + separator.size());
	}
	cont.insert(cont.end(), work);
	return cont;
}

/**
 * \brief URL related stuff
 */
class URL : public std::vector<std::string> {
	std::string	_method;
public:
	const std::string&	method() const { return _method; }
public:
	URL(const std::string& urlstring);
	operator std::string() const;
	static std::string	encode(const std::string& in);
	static std::string	decode(const std::string& in);
};

/**
 * \brief Method to absorb characters from a stream
 *
 * This method is very often used when parsing.
 */
void	absorb(std::istream& in, char c);

/**
 * \brief Locker class
 *
 * We want to make sure the pthread_mutex is unlocked when an exception
 * is thrown, so we have to encapsulate the locking operation in a
 * class that locks when the object is created an unlocks then it is
 * destroyed.
 */
class PthreadLocker {
	pthread_mutex_t *_lock;
public:
	PthreadLocker(pthread_mutex_t *lock, bool blocking = true);
	~PthreadLocker();
};

/**
 * \brief Mixin class for type name information
 */
class Typename {
public:
	virtual std::string	type_name() const;
};

/**
 * \brief Remove white space at the beginning and end of a string
 */
std::string	trim(const std::string& s);

/**
 * \brief Remove white space at the end of a string
 */
std::string	rtrim(const std::string& s);

/**
 * \brief Remove white space at the begining of a string
 */
std::string	ltrim(const std::string& s);

/**
 * \brief Format a time stamp
 *
 * The strftime function is used on a struct tm to format Unix timestamps.
 * However, using strftime is mildly annoying, localtime needs  time_t
 * pointer, not a time_t value, so it cannot in some cases, extra code
 * needs to be written just to get that pointer. Furthermore, strftime
 * writes to a buffer, in C++ we would prefer to work with a std::string.
 * This is what this function provides. It formats a time_t value as
 * a timestamp using the strftime format specification in the first
 * argument.
 *
 * \param format	strftime format specification
 * \param when		Unix time value to format
 * \param local		whether or not local time should be used for the
 *			format. If false, GMT is used.
 */
std::string	timeformat(const std::string& format, time_t when,
			bool local = true);

/**
 * \brief Attribute value pairs container
 *
 * Command line applications use arguments of the form attribute=value
 * instead of position arguments to simplify matters for users. This
 * class provides a method to parse a vector of such attribute value
 * strings into a map of attribute value pairs.
 */
class AttributeValuePairs {
public:
	typedef std::pair<std::string, std::string>	pair_t;
	typedef std::multimap<std::string, std::string>	map_t;
private:
	map_t	data;
	pair_t	parse(const std::string& argument) const;
public:
	AttributeValuePairs();
	AttributeValuePairs(const std::vector<std::string>& arguments,
		int skip = 0);
	bool	has(const std::string& attribute) const;
	std::string	operator()(const std::string& attribute) const;
	std::set<std::string>	get(const std::string& attribute) const;
};

/**
 * \brief Universally unique id used to tell images appart
 *
 * Images created by the system are tagged with UUIDs so that copies can
 * easily be detected as equal.
 */
class UUID {
	std::string	_uuid;
public:
	UUID();
	UUID(const std::string& uuid);
	bool	operator==(const UUID& other) const;
	operator	std::string() const;
};
std::ostream&	operator<<(std::ostream& out, const UUID& uuid);

/**
 * \brief Path encoding 
 */
class Path : public std::vector<std::string> {
public:
	Path(const std::string& path);
	std::string	basename() const;
	std::string	dirname() const;
	bool	isAbsolute() const;
};

/**
 * \brief Demangling of symbols and type names if available
 */
std::string	demangle(const std::string& mangled_name);

} // namespace astro

#endif /* _AstroUtils_h */
