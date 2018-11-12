/*
 * NodePaths.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>

namespace astro {
namespace process {

std::string	NodePaths::srcfile(const std::string& file) const {
	return _srcpath->file(file);
}

std::string	NodePaths::dstfile(const std::string& file) const {
	return _dstpath->file(file);
}

} // namespace process
} // namespace astro

