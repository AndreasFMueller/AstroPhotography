/*
 * Time.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroUtils.h>

namespace astro {

/**
 * \brief Construct a Time object for now
 */
Time::Time() {
	::time(&_time);
}

/**
 * \brief Construct a Time object for some other time
 */
Time::Time(time_t t) {
	_time = t;
}

/**
 * \brief Format a time string
 *
 * \param format	format string to use
 * \param local		whether to display as local time
 */
std::string	Time::toString(const std::string& format, bool local) const {
	return timeformat(format, _time, local);
}

/**
 * \brief Format a time string
 *
 * \param format	format string to use
 * \param local		whether to display as local time
 */
std::string	Time::toString(const char *format, bool local) const {
	return timeformat(format, _time, local);
}

/**
 * \brief Format a time string
 *
 * This uses the default format "%T %F"
 *
 * \param local		whether to display as local time
 */
std::string	Time::toString(bool local) const {
	return timeformat("%T %F", _time, local);
}

} // namespace astro
