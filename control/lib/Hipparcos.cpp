/*
 * Hipparcos.cpp -- Hipparcos catalog access implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Hipparcos.h>
#include <AstroDebug.h>
#include <includes.h>
#include <AstroFormat.h>

namespace astro {
namespace catalog {

//////////////////////////////////////////////////////////////////////
// Hipparcos star implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Construct a Hipparcos star from a line in the Hipparcos catalog
 *
 * \param line	The line is a 451 character long record containing the star
 *		information in text form
 */
HipparcosStar::HipparcosStar(const std::string& line) {
	hip = std::stoi(std::string(line, 8, 6));
	ra().hours(std::stoi(line.substr(17, 2))
		+ std::stoi(line.substr(20, 2)) / 60.
		+ std::stod(line.substr(23, 5)) / 3600.);
	dec().degrees(((line[29] == '-') ? -1 : 1) *
		(std::stoi(line.substr(30, 2))
		 + std::stoi(line.substr(33, 2)) / 60.
		 + std::stod(line.substr(36, 4)) / 3600.));
	pm().ra().degrees(std::stod(line.substr(87, 8)) / 3600000.);
	pm().dec().degrees((std::stod(line.substr(96, 8)) / 3600000.)
		/ cos(dec()));
	mag() = std::stod(line.substr(41, 5));
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

//////////////////////////////////////////////////////////////////////
// Hipparcos catalog implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Create a Hipparcos catalog instance
 *
 * \param filename	the Hipparcos main database file, usually called
 *			hip_main.dat
 */
Hipparcos::Hipparcos(const std::string& filename) : MappedFile(filename, 451),
	_filename(filename) {
	int	skipped = 0;
	for (size_t recno = 0; recno < nrecords(); recno++) {
		std::string	record = get(recno);
		try {
			HipparcosStar	star(record);
			insert(std::make_pair(star.hip, star));
		} catch (...) {
			skipped++;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d records skipped", skipped);
}

/**
 * \brief Retrieve a star using the HIP number
 */
HipparcosStar	Hipparcos::find(unsigned int hip) const {
	const_iterator	s = std::map<unsigned int, HipparcosStar>::find(hip);
	if (s == end()) {
		throw std::runtime_error("illegal hip number");
	}
	return s->second;
}

/**
 * \brief Retrieve stars in a a window an not too faint
 */
Hipparcos::starsetptr	Hipparcos::find(const SkyWindow& window,
				const MagnitudeRange& magrange) const {
	starset	*result = new starset();
	starsetptr	resultptr(result);
	const_iterator	s;
	for (s = begin(); s != end(); s++) {
		if (window.contains(s->second)
			&& (magrange.contains(s->second.mag()))) {
			result->insert(s->second);
		}
	}
	return resultptr;
}


} // namespace catalog
} // namespace astro
