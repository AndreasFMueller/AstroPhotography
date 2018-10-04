/*
 * HipparcosStar.cpp -- Hipparcos star implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Hipparcos.h"
#include <AstroDebug.h>
#include <includes.h>
#include <AstroFormat.h>

namespace astro {
namespace catalog {

/**
 * \brief Construct a Hipparcos star from a line in the Hipparcos catalog
 *
 * \param line	The line is a 451 character long record containing the star
 *		information in text form
 */
HipparcosStar::HipparcosStar(const std::string& line)
	: Star(stringprintf("HIP%06u", std::stoi(std::string(line, 8, 6)))) {
	hip = std::stoi(std::string(line, 8, 6));
	catalog('H');
	catalognumber(hip);
	ra().hours(std::stoi(line.substr(17, 2))
		+ std::stoi(line.substr(20, 2)) / 60.
		+ std::stod(line.substr(23, 5)) / 3600.);
	dec().degrees(
		((line[29] == '-') ? (-1) : 1) *
		(std::stoi(line.substr(30, 2))
		 + std::stoi(line.substr(33, 2)) / 60.
		 + std::stod(line.substr(36, 4)) / 3600.));
	pm().ra().degrees(std::stod(line.substr(87, 8)) / 3600000.);
	pm().dec().degrees((std::stod(line.substr(96, 8)) / 3600000.)
		/ cos(dec()));
	mag(std::stod(line.substr(41, 5)));
}

std::string	HipparcosStar::toString() const {
	return stringprintf("HIP%u ", hip) + Star::toString();
}

bool	HipparcosStar::operator<(const HipparcosStar& other) const {
	return hip < other.hip;
}

bool	HipparcosStar::operator>(const HipparcosStar& other) const {
	return hip > other.hip;
}

bool	HipparcosStar::operator<=(const HipparcosStar& other) const {
	return hip <= other.hip;
}

bool	HipparcosStar::operator>=(const HipparcosStar& other) const {
	return hip >= other.hip;
}

} // namespace catalog
} // namespace astro
