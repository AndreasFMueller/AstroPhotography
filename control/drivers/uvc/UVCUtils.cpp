/*
 * UVCUtils.cpp -- utilities implementation for the UVC driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <UVCUtils.h>

namespace astro {
namespace camera {
namespace uvc {

UVCError::UVCError(const char *cause) : std::runtime_error(cause) {
}

} // namespace uvc
} // namespace camera
} // namespace astro
