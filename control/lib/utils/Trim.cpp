/*
 * Trim.cpp -- utility functions to trim strings
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>

namespace astro {

/**
 * \brief Remove white space at the beginning and end of a string
 */
std::string	trim(const std::string& s) {
	std::string	ss = rtrim(ltrim(s));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "trim('%s') = '%s'",
		s.c_str(), ss.c_str());
	return ss;
}

/**
 * \brief Remove white space at the end of a string
 */
std::string	rtrim(const std::string& s) {
	size_t	end = s.find_last_not_of(" \t\n");
	if (end == std::string::npos) {
		return std::string();
	}
	return s.substr(0, end + 1);
}

/**
 * \brief Remove white space at the begining of a string
 */
std::string	ltrim(const std::string& s) {
	size_t	start = s.find_first_not_of(" \t\n");
	if (start == std::string::npos) {
		return std::string();
	}
	return s.substr(start);
}

} // namespace astro
