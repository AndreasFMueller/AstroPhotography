/*
 * LongLat.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCoordinates.h>
#include <sstream>

namespace astro {

std::string	LongLat::toString() const {
	Angle	lo = longitude();
	while (lo > Angle(M_PI)) { lo = lo - Angle(M_PI); }
	while (lo < Angle(-M_PI)) { lo = lo + Angle(M_PI); }
	Angle	la = latitude();
	std::ostringstream	out;
	if (lo >= 0) {
		out << "E " << lo.dms();
	} else {
		out << "W " << (-lo).dms();
	}
	if (la >= 0) {
		out << " N " << la.dms();
	} else {
		out << " S " << (-la).dms();
	}
	return out.str();
}

} // namespace astro
