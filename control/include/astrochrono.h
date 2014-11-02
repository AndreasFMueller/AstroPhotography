/*
 * astrochrono.h -- type mapping for chrono libraries
 *
 * This header was necessary because g++ does not implement C++11 std::chrono,
 * so on Linux with g++, one needs to use the api compatible boost::chrono
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _astrochrono_h
#define _astrochrono_h

// first ensure that we read the configuration constants
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#if ENABLE_BOOSTREGEX

#include <boost/chrono.hpp>

namespace astro {

typedef	boost::chrono	chrono;

} // namespace astro

#else

#include <chrono>

namespace astro {

typedef	std::chrono	chrono;

} // namespace astro

#endif /* ENABLE_BOOSTREGEX */

#endif /* _astrochrono_h */
