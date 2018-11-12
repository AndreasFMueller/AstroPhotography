/*
 * StepPath.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <sys/stat.h>

namespace astro {
namespace process {

StepPath::StepPath(StepPathPtr parent) : _parent(parent) {
}

StepPath::StepPath(const std::string& p, StepPathPtr parent)
	: _parent(parent), _path(p) {
}

bool	StepPath::absolute(const std::string& s) const {
	if (s.size() == 0) return false;
	if (s[0] == '/') return true;
	if (s.size() > 1) {
		if ((s[0] == '.') && (s[1] == '/')) {
			return true;
		}
	}
	return false;
}

std::string	StepPath::dir() const {
	std::string	result = _path;
	if (absolute(_path)) {
		goto thatsit;
	}
	if (!_parent) {
		goto thatsit;
	}
	if (_path.size() > 0) {
		result = _parent->dir() + '/' + _path;
	} else {
		result = _parent->dir();
	}
thatsit:
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructed dir name: %s",
		result.c_str());
	return result;
}

std::string	StepPath::file(const std::string& file) const {
	std::string	result = file;
	if (file.size() == 0) {
		goto thatsit;
	}
	if (absolute(file)) {
		goto thatsit;
	}
	{
		std::string	d = dir();
		if (d.size() > 0) {
			result = d + "/" + file;
		}
	}
thatsit:
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructed file name: %s",
		result.c_str());
	return result;
}

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
