/*
 * FITSexception.cpp -- implementation of the FITS exception
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroIO.h>
#include <cerrno>
#include <cstring>

namespace astro {
namespace io {

static std::string	fitsexception(const std::string& cause,
	const std::string& filename, int e) {
	if (e > 0) {
		return stringprintf("%s, file='%s', %s (%d)",
			cause.c_str(), filename.c_str(), strerror(e), e);
	}
	return stringprintf("%s, file='%s'", cause.c_str(), filename.c_str());
}

FITSexception::FITSexception(const std::string& cause,
	const std::string& filename)
	: std::runtime_error(fitsexception(cause, filename, -1)) {
}

FITSexception::FITSexception(const std::string& cause,
	const std::string& filename, int errno)
	: std::runtime_error(fitsexception(cause, filename, errno)) {
}

} // namespace io
} // namespace astro
