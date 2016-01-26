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
#include <list>
#include <map>
#include <AstroDebug.h>
#include <mutex>
#include <cstdlib>
#include <iostream>
#include <syslog.h>
#include <cstdio>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <memory>
#include <thread>

#ifdef __cplusplus
extern "C" {
#endif

extern void	syslog_stacktrace(int sig);
extern void	stderr_stacktrace(int sig);

#ifdef __cplusplus
}
#endif

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
 * \brief BlockStopWatch class
 *
 * This is intended to be used for performance measurements only, and only
 * during debugging. It measures the elased and CPU time between creation and
 * distruction and displays a debug message with the results when it is
 * destroyed. This allows for a simple way to measure the time spent in a
 * block.
 */
class BlockStopWatch {
	std::string	_message;
	struct timeval	start_time;
	struct rusage	start_usage;
public:
	BlockStopWatch(const std::string& message);
	~BlockStopWatch();
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
 * \brief Method to absorb characters from a stream
 *
 * This method is very often used when parsing.
 */
void	absorb(std::istream& in, char c);

/**
 * \brief Mutex locker class
 *
 * Does essentially the same as the pthread mutex locker class, but for
 * C++11 Lockables
 */
template<typename Lockable>
class MutexLocker {
	Lockable&	_mtx;
private:
	// prevent copying of the locker class
	MutexLocker(const MutexLocker& other);
	//MutexLocker&	MutexLocker::operator=(const MutexLocker& other);
public:
	MutexLocker(Lockable& mtx, bool blocking = true) : _mtx(mtx) {
		if (blocking) {
			_mtx.lock();
			return;
		}
		if (!_mtx.try_lock()) {
			throw std::runtime_error("cannot lock");
		}
	}
	~MutexLocker() {
		_mtx.unlock();
	}
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
	AttributeValuePairs(const std::list<std::string>& arguments,
		int skip = 0);
	bool	has(const std::string& attribute) const;
	std::string	operator()(const std::string& attribute) const;
	std::set<std::string>	get(const std::string& attribute) const;
	void	erase(const std::string& attribute);
	std::set<std::string>	attributes() const;
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
	bool	operator<(const UUID& other) const;
	UUID&	operator=(const UUID& other);
	operator	std::string() const;
};
std::ostream&	operator<<(std::ostream& out, const UUID& uuid);

/**
 * \brief Path encoding 
 */
class Path : public std::vector<std::string> {
public:
	Path() { }
	Path(const std::string& path);
	std::string	basename() const;
	std::string	dirname() const;
	bool	isAbsolute() const;
};

/**
 * \brief Demangling of symbols and type names if available
 */
std::string	demangle(const std::string& mangled_name) throw();

/**
 * \brief a class that handles parsing server names with attached ports
 */
class ServerName {
	std::string	_host;
public:
	std::string	host() const;
	void	host(const std::string& h) { _host = h; }
private:
	unsigned short	_port;
public:
	unsigned short	port() const;
	void	port(const unsigned short p) { _port = p; }
private:
	bool	_isdynamic; 
public:
	bool	isdynamic() const { return _isdynamic; }

public:
	std::string	connect(const std::string& service) const;

	// constructor
	ServerName();
	ServerName(const std::string& _host, unsigned short port);
	ServerName(const std::string& dynamicname);

	bool	isDefault() const;
	bool	isDefaultPort() const;
	operator	std::string() const;
	bool	operator==(const ServerName& other) const;
	bool	operator!=(const ServerName& other) const;

	std::string	toString() const;
};

/**
 * \brief URL related stuff
 */
class URL : public ServerName, public Path {
	std::string	_method;
public:
	const std::string&	method() const { return _method; }
public:
	URL(const std::string& urlstring);
	operator std::string() const;
	static std::string	encode(const std::string& in);
	static std::string	decode(const std::string& in);
	std::string	path() const;
};


/**
 * \brief A template to unify what we do in the main function of all programs
 */
template <int mainfunction(int, char *[])>
int	main_function(int argc, char *argv[]) {
	signal(SIGSEGV, stderr_stacktrace);
	try {
		return mainfunction(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << Path(argv[0]).basename();
		std::cerr << " terminated by ";
		std::cerr << astro::demangle(typeid(x).name());
		std::cerr << ": " << x.what();
		std::cerr << std::endl;
	} catch (...) {
		std::cerr << Path(argv[0]).basename();
		std::cerr << " terminated by unknown exception";
		std::cerr << std::endl;
	}
	return EXIT_FAILURE;
}

/**
 * \brief Pidfile class, creates a pid file, writes the pid to it
 *
 * when an instance of this class goes out of scope, the pid file is
 * removed.
 */
class PidFile {
	std::string	_filename;
public:
	PidFile(const std::string& filename);
	~PidFile();
};

/**
 * \brief Interface class for actions
 */
class Action {
public:
	virtual void	execute() = 0;
};
typedef std::shared_ptr<Action>	ActionPtr;

/**
 * \brief Asynchronous action class
 *
 * This class asynchronously executes an action unless there already is
 * an action executing.
 */
class AsynchronousAction {
	std::thread	worker;
	ActionPtr	_action;
	bool		_busy;
	std::mutex	mtx;

	AsynchronousAction(const AsynchronousAction& other);
	AsynchronousAction&	operator=(const AsynchronousAction& other);

	void	busy(bool b);
public:
	AsynchronousAction();
	~AsynchronousAction();
	bool	execute(ActionPtr action);
	void	execute();
	
};

} // namespace astro

#endif /* _AstroUtils_h */
