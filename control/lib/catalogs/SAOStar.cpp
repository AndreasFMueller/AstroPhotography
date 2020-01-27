/*
 * SAOStar.cpp -- SAO star implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "SAO.h"
#include <AstroDebug.h>
#include <includes.h>
#include <AstroFormat.h>

namespace astro {
namespace catalog {

/**
 * \brief Construct a SAO star from a line in the SAO catalog
 *
 * \param line	The line is a 451 character long record containing the star
 *		information in text form
 */
SAOStar::SAOStar(const std::string& line)
	: Star(stringprintf("SAO%06u", std::stoi(std::string(line, 0, 6)))) {
	sao = std::stoi(std::string(line, 0, 6));
	catalog('S');
	catalognumber(sao);
	ra().hours(std::stoi(line.substr(150, 2))
		+ std::stoi(line.substr(152, 2)) / 60.
		+ std::stod(line.substr(154, 6)) / 3600.);
	dec().degrees(
		((line[167] == '-') ? (-1) : 1) *
		(std::stoi(line.substr(168, 2))
		 + std::stoi(line.substr(170, 2)) / 60.
		 + std::stod(line.substr(172, 5)) / 3600.));
	pm().ra().degrees(std::stod(line.substr(160, 7)) / 3600000.);
	pm().dec().degrees((std::stod(line.substr(177, 6)) / 3600000.)
		/ cos(dec()));
	mag(std::stod(line.substr(80, 4)));
}

std::string	SAOStar::toString() const {
	return stringprintf("SAO%u ", sao) + Star::toString();
}

bool	SAOStar::operator<(const SAOStar& other) const {
	return sao < other.sao;
}

bool	SAOStar::operator>(const SAOStar& other) const {
	return sao > other.sao;
}

bool	SAOStar::operator<=(const SAOStar& other) const {
	return sao <= other.sao;
}

bool	SAOStar::operator>=(const SAOStar& other) const {
	return sao >= other.sao;
}

} // namespace catalog
} // namespace astro
