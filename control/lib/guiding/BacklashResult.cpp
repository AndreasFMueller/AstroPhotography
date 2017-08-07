/*
 * BacklashResult.cpp -- implementation of backlash result operations
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <Backlash.h>
#include <sstream>

namespace astro {
namespace guiding {

/**
 * \brief Convert the result into a string
 */
std::string	BacklashResult::toString() const {
	std::ostringstream	o;
	o << *this;
	return o.str();
}

/**
 * \brief Write the result to a stream
 */
std::ostream&	operator<<(std::ostream& out, const BacklashResult& br) {
	switch (br.direction) {
	case backlash_dec: out << "DEC"; break;
	case backlash_ra: out << "RA"; break;
	}
	out << ", ";
	out << "x=" << br.x << ", ";
	out << "y=" << br.y << ", ";
	out << "long=" << br.longitudinal << ", ";
	out << "lat=" << br.lateral << ", ";
	out << "forward=" << br.forward << ", ";
	out << "backward=" << br.backward << ", ";
	out << "f=" << br.f << ", ";
	out << "b=" << br.b << ", ";
	out << "offset=" << br.offset << ", ";
	out << "drift=" << br.drift;
	return out;
}

double	BacklashResult::operator()(const int k[4], const BacklashPoint& p) {
	return    k[0] * f
		+ k[1] * forward
		- k[2] * b
		- k[3] * backward
		+ offset + drift * p.time;
}

} // namespace guiding
} // namespace astro
