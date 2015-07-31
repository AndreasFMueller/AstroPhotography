/*
 * Ucac4StarNumber.cpp -- Ucac4 star names
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Ucac4.h"
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <includes.h>

namespace astro {
namespace catalog {

Ucac4StarNumber::Ucac4StarNumber(const std::string& starnumber) {
	if (2 != sscanf(starnumber.c_str(), "UCAC4-%hu-%u",
		&_zone, &_number)) {
		std::string	msg = stringprintf("cannot parse UCAC4 star "
			"number '%s'", starnumber.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}

std::string	Ucac4StarNumber::toString() const {
	return stringprintf("UCAC4-%03hu-%06u", _zone, _number);
}

bool	Ucac4StarNumber::operator==(const Ucac4StarNumber& other) const {
	return (_zone == other._zone) && (_number == other._number);
}

bool	Ucac4StarNumber::operator!=(const Ucac4StarNumber& other) const {
	return !(*this == other);
}

bool	Ucac4StarNumber::operator<(const Ucac4StarNumber& other) const {
	if (_zone < other._zone) {
		return true;
	}
	if (_zone > other._zone) {
		return false;
	}
	return (_number < other._number);
}

std::ostream&	operator<<(std::ostream& out, const Ucac4StarNumber& star) {
	out << star.toString();
	return out;
}

} // namespace catalog
} // namespace astro
