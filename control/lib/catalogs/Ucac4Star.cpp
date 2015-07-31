/*
 * Ucac4Star.cpp -- Ucac4Star implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Ucac4.h"
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <includes.h>

namespace astro {
namespace catalog {

std::string	Ucac4Star::toString() const {
	return stringprintf("%s %8.4f %8.4f %6.3f", number.toString().c_str(),
		ra().hours(), dec().degrees(), mag());
}

Ucac4Star::Ucac4Star(const std::string& starnumber)
	: Star(starnumber), number(starnumber) {
}

Ucac4Star::Ucac4Star(uint16_t zone, uint32_t _number)
	: Star(Ucac4StarNumber(zone, _number).toString()),
	  number(zone, _number) {
}

bool	Ucac4Star::operator<(const Ucac4Star& other) const {
	return number < other.number;
}

} // namespace catalog
} // namespace astro
