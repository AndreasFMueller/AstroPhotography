/*
 * SxUtils.h -- utilities for 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxUtils_h
#define _SxUtils_h

#include <stdexcept>

namespace astro {
namespace camera {
namespace sx {

class SxError : public std::runtime_error {
public:
	SxError(const char *cause);
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxUtils_h */
