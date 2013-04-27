/*
 * UVCUtils.h -- utilities for the UVC driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _UVCUtils_h
#define _UVCUtils_h

#include <stdexcept>

namespace astro {
namespace camera {
namespace uvc {

class	UVCError : public std::runtime_error {
public:
	UVCError(const char *cause);
};

} // namespace uvc
} // namespace camera
} // namespace astro

#endif /* _UVCUtils_h */
