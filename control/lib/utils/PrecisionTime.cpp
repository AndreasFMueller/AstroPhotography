/*
 * PrecisionTime.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <regex>

namespace astro {

PrecisionTime::PrecisionTime() {
	gettimeofday(&_tv, NULL);
}

PrecisionTime::PrecisionTime(time_t t) {
	_tv.tv_sec = t;
	_tv.tv_usec = 0;
}

PrecisionTime::PrecisionTime(const struct timeval& tv) {
	_tv.tv_sec = tv.tv_sec;
	_tv.tv_usec = tv.tv_usec;
}

time_t  PrecisionTime::time() const {
	return _tv.tv_sec;
}

void    PrecisionTime::time(time_t t) {
	_tv.tv_sec = t;
	_tv.tv_usec = 0;
}

std::string     PrecisionTime::toString(const std::string& format,
	bool local) const {
	// look for %[0-9.]*f, use that to format the microseconds
	std::smatch	m;
	std::regex	r("(.*)(%[0-9.]*f)(.*)", std::regex::extended);
	if (!std::regex_match(format, m, r)) {
		return timeformat(format, _tv.tv_sec, local);
	}

	// extract the match and use it to format the microseconds
	if (m.size() != 4) {
		return timeformat(format, _tv.tv_sec, local);
	}
	std::string	microformat = m[2].str();
	std::string	microseconds
		= stringprintf(microformat, _tv.tv_usec / 1000000.).substr(2);

	// replace the match by 
	std::string	f2 = m[1].str() + microseconds + m[3].str();

	// use the remainder to format the time
	return timeformat(f2, _tv.tv_sec, local);
}

std::string	PrecisionTime::toString(const char *format, bool local) const {
	return toString(std::string(format), local);
}

std::string     PrecisionTime::toString(bool local) const {
	return stringprintf("%s.%03d",
		timeformat("%F %T", _tv.tv_sec, local).c_str(),
		_tv.tv_usec / 1000);
}

} // namespace astro
