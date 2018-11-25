/*
 * NodePaths.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <sstream>
#include <AstroDebug.h>

namespace astro {
namespace process {

/**
 * \brief Default Construct the NodePaths with no paths
 */
NodePaths::NodePaths() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "default construct NodePaths");
}

/**
 * \brief Copy construct a NodePaths object
 */
NodePaths::NodePaths(NodePaths& other) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "copying paths: %s",
		other.NodePaths::info().c_str());
	_srcpath = other._srcpath;
	_dstpath = other._dstpath;
}

/**
 * \brief Construct a source file name
 *
 * \param file		filename to 
 */
std::string	NodePaths::srcfile(const std::string& file) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "srcfile from '%s' and '%s'",
		(_srcpath) ? _srcpath->path().c_str() : "(null)", file.c_str());
	if (_srcpath) {
		return _srcpath->file(file);
	}
	return file;
}

/**
 * \brief Construct a destination file name
 *
 * \param file		filename
 */
std::string	NodePaths::dstfile(const std::string& file) const {
	if (_dstpath) {
		return _dstpath->file(file);
	}
	return file;
}

/**
 * \brief Construct an info string
 */
std::string	NodePaths::info() const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing NodePaths::info()");
	std::ostringstream	out;
	if (srcpath()) {
		out << " src=" << srcpath()->path();
	} else {
		out << " src=nil";
	}
	if (dstpath()) {
		out << " dst=" << dstpath()->path();
	} else {
		out << " dst=nil";
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "NodePaths::info() complete");
	return out.str();
}

} // namespace process
} // namespace astro

