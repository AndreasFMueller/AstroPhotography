/*
 * variables.cpp -- common method implementations
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <variables.h>
#include <iostream>

namespace astro {
namespace cli {

std::ostream&   operator<<(std::ostream& out, const ValuePtr varp) {
	out << &*varp;
	return out;
}

std::ostream&	operator<<(std::ostream& out, const variables& table) {
	variables::const_iterator	i;
	for (i = table.begin(); i != table.end(); i++) {
		out << i->first << " = ";
		out << *((value<double>*)&*(i->second))->val();
		out << std::endl;
	}
	return out;
}

bool	variables::contains(const std::string& name) const {
	return (end() != find(name));
}

} // namespace cli
} // namespace astro
