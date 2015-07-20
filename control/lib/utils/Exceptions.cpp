/*
 * Exceptions.cpp -- implementation of some of the exceptions
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroExceptions.h>
#include <AstroFormat.h>
#include <includes.h>

namespace astro {

runtime_errno::runtime_errno(const char *cause, int unix_errno)
	: std::runtime_error(stringprintf("%s: %s", cause, strerror(unix_errno))),
	  _unix_errno(unix_errno) {
}

} // namespace astro
