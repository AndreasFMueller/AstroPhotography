/*
 * utils.h - utility functions for the SBIG camera driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _utils_h
#define _utils_h

#include <stdexcept>

namespace astro {
namespace camera {
namespace sbig {

std::string    sbig_error(short errorcode);

class SbigError : public std::runtime_error {
public:
	SbigError(short errocode);
	SbigError(const char *cause);
};

} // namespace sbig
} // namespace camera
} // namespace astro

#endif /* _utils_h */
