/*
 * SxUtils.cpp -- starlight express utilities 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxUtils.h>

namespace astro {
namespace camera {
namespace sx {

SxError::SxError(const char *cause) : std::runtime_error(cause) {
}

} // namespace sx
} // namespace camera
} // namespace astro
