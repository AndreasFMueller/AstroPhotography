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

#if ENABLE_BOOSTCHRONO

#include <boost/chrono.hpp>

namespace astro {
namespace chrono {

typedef	boost::chrono::steady_clock	steady_clock;
typedef	boost::chrono::milliseconds	milliseconds;

} // namespace chrono
} // namespace astro

#else

#include <chrono>

namespace astro {
namespace chrono {

typedef	std::chrono::steady_clock	steady_clock;
typedef	std::chrono::milliseconds	milliseconds;

} // namespace chrono
} // namespace astro

#endif /* ENABLE_BOOSTCHRONO */

#endif /* _astrochrono_h */
