/*
 * FITSfile.cpp -- implementation of FITS io routines
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
 * \brief Whether or not this filename is a FITS filename
 */
bool	FITSfile::isname(const std::string& filename) {
	if (filename.size() < 5) { return false; }
	return (filename.substr(filename.size() - 5) == std::string(".fits"));
}

/**
 * \brief Retrieve a human readable error message from the fits library
 */
std::string	FITSfile::errormsg(int status) const {
	char	errmsg[128];
	fits_get_errstatus(status, errmsg);
	return std::string(errmsg);
}

/**
 * \brief Construct a FITS file object
 *
 * This does not open a file, which is reserved to the derived classes.
 */
FITSfile::FITSfile(const std::string& _filename,
	int _pixeltype, int _planes, int _imgtype)
	: filename(_filename), fptr(NULL), pixeltype(_pixeltype),
	  planes(_planes), imgtype(_imgtype) {
}

/**
 * \brief Destroy a FITS file.
 *
 * This destructor closes the file, if it is open
 */
FITSfile::~FITSfile() {
	if (NULL == fptr) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s: no FITS fptr to close",
			filename.c_str());
		return;
	}
	int	status = 0;
	if (fits_close_file(fptr, &status)) {
		// XXX what do I do if the close fails?
		// throw FITSexception(errormsg(status));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "close FITS file %s",
		filename.c_str());
	fptr = NULL;
}

/**
 * \brief Auxiliary predicate class to find headers
 */
class match_header_name {
	std::string	_name;
public:
	match_header_name(const std::string& name) : _name(name) { }
	bool	operator()(const std::pair<std::string, FITShdu>& v) const {
		return v.first == _name;
	}
};

/**
 * \brief Header access
 */
FITSfile::headerlist::const_iterator	FITSfile::find(const std::string& name) const {
	return std::find_if(headers.begin(), headers.end(),
		match_header_name(name));
}

FITSfile::headerlist::iterator	FITSfile::find(const std::string& name) {
	return std::find_if(headers.begin(), headers.end(),
		match_header_name(name));
}

bool	FITSfile::hasHDU(const std::string& keyword) const {
	return find(keyword) != headers.end();
}

const FITShdu&	FITSfile::getHDU(const std::string& keyword) const {
	if (!hasHDU(keyword)) {
		std::string	msg = stringprintf("no header with keyword %s",
			keyword.c_str());
		throw std::runtime_error(msg);
	}
	return find(keyword)->second;
}

/**
 * \brief metadata access
 */
bool	FITSfile::hasMetadata(const std::string& keyword) const {
	return hasHDU(keyword);
}

Metavalue	FITSfile::getMetadata(const std::string& keyword) const {
	return FITSKeywords::meta(getHDU(keyword));
}

ImageMetadata	FITSfile::getAllMetadata() const {
	ImageMetadata	meta;
	headerlist::const_iterator	hi;
	for (hi = headers.begin(); hi != headers.end(); hi++) {
		meta.setMetadata(FITSKeywords::meta(hi->second));
	}
	return meta;
}

} // namespace io
} // namespace astro

