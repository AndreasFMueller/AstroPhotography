/*
 * utils.cpp -- utility functions
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroGuiding.h>
#include <AstroUtils.h>
#include <iostream>

namespace astro {

/**
 * \brief Auxiliary function for parsing
 *
 * This function reads a single character from the input stream and checks
 * whether it is what we expect it to be. If not, it throws an exception.
 */
void	absorb(std::istream& in, char b) {
	char	c;
	in >> c;
	if (b != c) {
		debug(LOG_ERR, DEBUG_LOG, 0, "expected %c, found %c", b, c);
		throw std::runtime_error("illegal character");
	}
}

} // namespace astro
