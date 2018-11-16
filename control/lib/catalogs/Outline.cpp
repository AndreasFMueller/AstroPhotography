/*
 * Outline.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <sstream>

namespace astro {
namespace catalog {

std::string	Outline::toString() const {
	std::ostringstream	out;
	out << _name << ":";
	std::for_each(begin(), end(),
		[&](const astro::RaDec& radec) {
			out << " " << radec.toString();
		}
	);
	return out.str();
}

} // namespace catalog
} // namespace astro
