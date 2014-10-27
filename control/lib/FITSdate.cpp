/*
 * FITSdate.cpp -- abstraction for dates in FITS headers
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <AstroFormat.h>
#include <includes.h>
#include <regex>

namespace astro {
namespace image {

/**
 * \brief Create an FITSdate object from a FITS formatted date specification
 */
FITSdate::FITSdate(const std::string& date) {
	std::string	r("([0-9]{4})-([0-9]{2})-([0-9]{2})"
		"(T([0-9]{2}):([0-9]{2}):([0-9]{2})(\\.([0-9]{3})){0,1}){0,1}");
	std::regex	regex(r, std::regex::extended);
	std::smatch	matches;

	if (!std::regex_match(date, matches, regex)) {
		std::string	msg = stringprintf("bad FITSdate '%s'",
			date.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	struct tm	result;

	// year
	result.tm_year = std::stoi(matches[1]) - 1900;

	// month
	result.tm_mon = std::stoi(matches[2]) - 1;

	// day
	result.tm_mday = std::stoi(matches[3]);

	// hour
	result.tm_hour = (matches.position(5) < 0) ? 0 : std::stoi(matches[5]);

	// minutes
	result.tm_min = (matches.position(6) < 0) ? 0 : std::stoi(matches[6]);

	// seconds
	result.tm_sec = (matches.position(7) < 0) ? 0 : std::stoi(matches[7]);

	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"year=%d, month=%d, day=%d, hour=%d, min=%d, sec=%d",
		result.tm_year, result.tm_mon, result.tm_mday,
		result.tm_hour, result.tm_min, result.tm_sec);

	// remaining fields
	result.tm_isdst = 0;
	result.tm_zone = NULL;
	result.tm_gmtoff = 0;

	// convert to unix time
	when.tv_sec = timegm(&result);
	when.tv_usec = 0;
	if ((matches.position(9) > 0) && (matches.length(9) > 0)) {
		when.tv_usec = 1000 * std::stoi(matches[9]);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "time: %d.%06d", when.tv_sec,
		when.tv_usec);
}

/**
 * \brief Create a FITSdate from unix time with usecs
 */
FITSdate::FITSdate(const struct timeval& tv) : when(tv) {
}

/**
 * \brief Create a FITSdate object from unix time
 */
FITSdate::FITSdate(time_t t) {
	when.tv_sec = t;
	when.tv_usec = 0;
}

/**
 * \brief CReate a FITSdate object representing the current time
 */
FITSdate::FITSdate() {
	gettimeofday(&when, NULL);
}

/**
 * \brief Convert a FITSdate to a date string
 */
std::string	FITSdate::showShort() const {
	struct tm	result;
	gmtime_r(&when.tv_sec, &result);
	char	b[128];
	strftime(b, sizeof(b), "%Y-%m-%d", &result);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "short form: %s", b);
	return std::string(b);
}

/**
 * \brief Convert a FITSdate to a full datetime string
 */
std::string	FITSdate::showLong() const {
	struct tm	result;
	gmtime_r(&when.tv_sec, &result);
	char	b[128];
	strftime(b, sizeof(b), "%Y-%m-%dT%H:%M:%S", &result);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "long form: %s", b);
	return std::string(b);
}

/**
 * \brief Convert a FITSdate to a full date string including milliseconds
 */
std::string	FITSdate::showVeryLong() const {
	struct tm	result;
	gmtime_r(&when.tv_sec, &result);
	char	b[128];
	strftime(b, sizeof(b), "%Y-%m-%dT%H:%M:%S", &result);
	std::string	ts = std::string(b)
				+ stringprintf(".%03d", when.tv_usec / 1000);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "very long form: %s", ts.c_str());
	return ts;
}

/**
 * \brief Equality operator
 */
bool	FITSdate::operator==(const FITSdate& other) const {
	return (when.tv_sec == other.when.tv_sec) 
		&& (when.tv_usec == other.when.tv_usec);
}

/**
 * \brief Comparison operator
 */
bool	FITSdate::operator<(const FITSdate& other) const {
	if (when.tv_sec == other.when.tv_sec) {
		return when.tv_usec < other.when.tv_usec;
	}
	return when.tv_sec < other.when.tv_sec;
}

} // namespace image
} // namespace astro
