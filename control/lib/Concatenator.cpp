/*
 * Contatenator.cpp -- concatenator functor class implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <algorithm>
#include <AstroDebug.h>
#include <iostream>

namespace astro {

void	Concatenator::operator()(const std::string& component) {
	if (_componentcount++) {
		_result.append(_separator);
	}
	_result.append(component);
}

std::string	Concatenator::concat(const std::vector<std::string>& v,
	const std::string& separator) {
	return std::for_each(v.begin(), v.end(), Concatenator(separator));
}

std::string	Concatenator::concat(const std::set<std::string>& v,
	const std::string& separator) {
	return std::for_each(v.begin(), v.end(), Concatenator(separator));
}

} // namespace astro
