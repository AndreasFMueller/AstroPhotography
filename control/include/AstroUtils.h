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
#include <AstroDebug.h>

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

} // namespace astro

#endif /* _AstroUtils_h */
