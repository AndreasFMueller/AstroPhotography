/*
 * FITShdu.cpp -- implementation of FITS io routines
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroIO.h>
#include <fitsio.h>
#include <AstroDebug.h>
#include <includes.h>
#include <AstroFormat.h>

using namespace astro::image;

namespace astro {
namespace io {

/**
 * \brief remove quotation marks from a string if present
 */
std::string	FITShdu::unquote(const std::string& s) {
	if ((s[0] == '\'') && (s[s.size() - 1] == '\'')) {
		return s.substr(1, s.size() - 2);
	}
	return s;
}

} // namespace io
} // namespace astro

