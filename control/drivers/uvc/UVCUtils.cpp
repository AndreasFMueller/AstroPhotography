/*
 * UvcUtils.cpp -- utilities implementation for the UVC driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <UvcUtils.h>

namespace astro {
namespace camera {
namespace uvc {

UvcError::UvcError(const char *cause) : std::runtime_error(cause) {
}

} // namespace uvc
} // namespace camera
} // namespace astro
