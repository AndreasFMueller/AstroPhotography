/*
 * astroregex.h -- type mapping for regex libraries
 *
 * This header was necessary because g++ does not implement C++11 std::regex,
 * so on Linux with g++, one needs to use the api compatible boost::regex
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _astroregex_h
#define _astroregex_h

// first ensure that we read the configuration constants
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#if ENABLE_BOOSTREGEX

#include <boost/regex.hpp>

namespace astro {

typedef	boost::regex	regex;
typedef boost::smatch	smatch;
using boost::regex_match;

} // namespace astro

#else
#error "std regex library"

#include <regex>

namespace astro {

typedef	std::regex	regex;
typedef std::smatch	smatch;
using std::regex_match;

} // namespace astro

#endif /* ENABLE_BOOSTREGEX */

#endif /* _astroregex_h */
