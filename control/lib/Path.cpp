/*
 * Path.cpp -- class to process path names
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>

namespace astro {

/**
 * \brief split the path into a vector of path components
 */
Path::Path(const std::string& path) {
	// split at slashes
	split<std::vector<std::string> >(path, std::string("/"), *this);
	// remove empty components at the end, but not at the
	// beginning. Empty component at the beginning means this
	// is an absolute path
	while ((rbegin() != rend()) && (rbegin()->size() == 0)) {
		pop_back();
	}
	// make sure the path is not empty
	if (size() == 0) {
		throw std::runtime_error("empty path");
	}
}

/**
 * \brief Find out whether this path is absolute
 */
bool	Path::isAbsolute() const {
	return (begin()->size() == 0);
}

/**
 * \brief Get the base name 
 */
std::string	Path::basename() const {
	return *rbegin();
}

/**
 * \brief Get the directory name
 *
 * If the path does not have any path components, then an exception is thrown
 */
std::string	Path::dirname() const {
	if (size() <= 1) {
		throw std::runtime_error("no path present, only base name");
	}
	auto	ptr = begin();
	std::string	dir = *ptr++;
	for (unsigned int i = 0; i < size() - 2; i++) {
		dir = dir + "/" + *ptr++;
	}
	return dir;
}

} // namespace astro
