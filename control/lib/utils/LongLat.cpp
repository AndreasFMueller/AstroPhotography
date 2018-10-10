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
		out << "E ";
	} else {
		out << "W ";
	}
	out	 << lo.dms().substr(1);
	if (la >= 0) {
		out << " N ";
	} else {
		out << " S ";
	}
	out	 << la.dms().substr(1);
	return out.str();
}

} // namespace astro
