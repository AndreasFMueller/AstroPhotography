/*
 * StepPath.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <sys/stat.h>
#include <string.h>

namespace astro {
namespace process {

StepPath::StepPath(StepPathPtr parent) {
	if (parent) {
		_path = parent->dir();
	}
}

StepPath::StepPath(const std::string& p, StepPathPtr parent) {
	if (parent_relative(p)) {
		if (parent) {
			_path = parent->dir() + "/" + p;
		} else {
			std::string	msg = stringprintf("cannot use parent "
				"relative path '%s' without parent", p.c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	} else {
		_path = p;
	}
}

bool	StepPath::absolute(const std::string& s) const {
	if (s.size() == 0) return false;
	return (s[0] == '/');
}

bool	StepPath::parent_relative(const std::string& s) const {
	return (!(absolute(s) || relative(s)));
}

bool	StepPath::relative(const std::string& s) const {
	if (s.size() <= 1) return false;
	return (s.substr(0,2) == std::string("./"));
}

/**
 * \brief Get the full path for the directory pointed to by this object
 */
std::string	StepPath::dir() const {
	return _path;
}

/**
 * \brief Construct a filename from the path
 *
 * \param file	name of the file
 */
std::string	StepPath::file(const std::string& file) const {
	// empty file names are not acceptable
	if (file.size() == 0) {
		std::string	msg = stringprintf("empty filename");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	std::string	result = file;

	// for a parent relative file, we use the parent step path to 
	// construct the file name
	if (parent_relative(file)) {
		std::string	d = dir();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "relative path from '%s' '%s'",
			d.c_str(), file.c_str());
		if (d.size() > 0) {
			result = d + "/" + file;
		}
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "file name constructed from '%s': '%s'",
		file.c_str(), result.c_str());
	return result;
}

/**
 * \brief Find out whether the directory exists
 */
bool	StepPath::direxists() const {
	struct stat	sb;
	std::string	name = dir();
	if (stat(name.c_str(), &sb) < 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot stat '%s': %s",
			name.c_str(), strerror(errno));
		return false;
	}
	if (!S_ISDIR(sb.st_mode)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is not a directory",
			name.c_str());
		return false;
	}
	return true;
}

/**
 * \brief Test whether a file exists
 *
 * \param f	Filename
 */
bool	StepPath::fileexists(const std::string& f) const {
	struct stat	sb;
	std::string	name = file(f);
	if (stat(name.c_str(), &sb) < 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot stat '%s': %s",
			name.c_str(), strerror(errno));
		return false;
	}
	if (!S_ISREG(sb.st_mode)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is not a file",
			name.c_str());
		return false;
	}
	return true;
}

} // namespace process
} // namespace astro
