/*
 * UvcUtils.h -- utilities for the UVC driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _UvcUtils_h
#define _UvcUtils_h

#include <stdexcept>

namespace astro {
namespace camera {
namespace uvc {

class	UvcError : public std::runtime_error {
public:
	UvcError(const char *cause);
};

} // namespace uvc
} // namespace camera
} // namespace astro

#endif /* _UvcUtils_h */
