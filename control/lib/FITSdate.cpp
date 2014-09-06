/*
 * FITSdate.cpp -- abstraction for dates in FITS headers
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <AstroFormat.h>
#include <includes.h>
#include <regex.h>

namespace astro {
namespace image {

/**
 * \brief Create an FITSdate object from a FITS formatted date specification
 */
FITSdate::FITSdate(const std::string& date) {
	int	rc = 0;
	const char	*r = "([0-9]{4})-([0-9]{2})-([0-9]{2})"
			"(T([0-9]{2}):([0-9]{2}):([0-9]{2})(\\.([0-9]{3})){0,1}){0,1}";
	debug(LOG_DEBUG, DEBUG_LOG, 0, "regular expression: %s", r);
	regex_t	regex;
	if (regcomp(&regex, r, REG_EXTENDED)) {
		throw std::runtime_error("internal error: RE does not compile");
	}
#define	nmatches	10
	regmatch_t	matches[nmatches];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "matching %s against %s",
		date.c_str(), r);
	rc = regexec(&regex, date.c_str(), nmatches, matches, 0);
	if (rc) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no match");
	}
	for (int i = 0; i < nmatches; i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "[%d]: %d - %d", i,
			matches[i].rm_so, matches[i].rm_eo);
	}
	struct tm	result;

	// year
	result.tm_year = std::stoi(date.substr(matches[1].rm_so,
		matches[1].rm_eo - matches[1].rm_so)) - 1900;

	// month
	result.tm_mon = std::stoi(date.substr(matches[2].rm_so,
		matches[2].rm_eo - matches[2].rm_so)) - 1;

	// day
	result.tm_mday = std::stoi(date.substr(matches[3].rm_so,
		matches[3].rm_eo - matches[3].rm_so));

	// hour
	result.tm_hour = (matches[5].rm_so < 0) ? 0
		: std::stoi(date.substr(matches[5].rm_so,
			matches[5].rm_eo - matches[5].rm_so));

	// minutes
	result.tm_min = (matches[6].rm_so < 0) ? 0
		: std::stoi(date.substr(matches[6].rm_so,
			matches[6].rm_eo - matches[6].rm_so));

	// seconds
	result.tm_sec = (matches[7].rm_so < 0) ? 0
		: std::stoi(date.substr(matches[7].rm_so,
			matches[7].rm_eo - matches[7].rm_so));

	// remaining fields
	result.tm_isdst = 0;
	result.tm_zone = NULL;
	result.tm_gmtoff = 0;

	// convert to unix time
	when.tv_sec = timegm(&result);
	when.tv_usec = 1000 * ((matches[9].rm_so < 0) ? 0
			: std::stoi(date.substr(matches[9].rm_so,
				matches[9].rm_eo - matches[9].rm_so)));
cleanup:
	regfree(&regex);
	if (rc) {
		std::string	msg = stringprintf("%s doesn't match date spec",
			date.c_str());
		throw std::runtime_error(msg);
	}
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


