/*
 * Tycho2.cpp -- Tycho2 star catalog class declarations
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Tycho2.h>
#include <includes.h>
#include <AstroFormat.h>
#include <AstroDebug.h>

namespace astro {
namespace catalog {

#define TYCHO2_RECORD_LENGTH	207

//////////////////////////////////////////////////////////////////////
// Tycho2 star class implementation
//////////////////////////////////////////////////////////////////////
void	Tycho2Star::setup(const std::string& line) {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "creating star from line '%s'",
	//	line.c_str());

	// check position flag
	if ('X' == line[13]) {
		throw std::runtime_error("record has no position");
	}

	// get magnitude
	float	vt = std::stod(line.substr(123, 6));
	float	bt = vt;
	try {
		bt = std::stod(line.substr(110, 6));
	} catch (...) { }
	_mag = vt - 0.090 * (bt - vt);

	// RA and DEC
	ra().degrees(std::stod(line.substr(15, 12)));
	dec().degrees(std::stod(line.substr(28, 12)));

	// proper motion
	pm().ra().degrees(std::stod(line.substr(41, 7)) / 3600000);
	pm().dec().degrees(std::stod(line.substr(49, 7)) / 3600000);

	// get the hipparcos number, if this is a hipparcos star
	try {
		_hip = std::stoi(line.substr(42, 6));
	} catch (...) {
		_hip = -1;
	}
}

Tycho2Star::Tycho2Star(int number, const std::string& line)
	: Star(stringprintf("T%d", number)) {
	setup(line);
}

//////////////////////////////////////////////////////////////////////
// Tycho2 catalog class implementation
//////////////////////////////////////////////////////////////////////

Tycho2::Tycho2(const std::string& filename)
	: MappedFile(filename, TYCHO2_RECORD_LENGTH), _filename(filename) {
}

/**
 * \brief get a star from the catalog
 */
Tycho2Star	Tycho2::find(unsigned int index) const {
	if (index >= nstars()) {
		throw std::runtime_error("not that many stars in Tycho2");
	}
	return Tycho2Star(index, get(index));
}

/**
 * \brief get all stars from a window
 */
Tycho2::starsetptr	Tycho2::find(const SkyWindow& window,
				const MagnitudeRange& magrange) const {
	starset	*result = new starset();
	starsetptr	resultptr(result);
	for (unsigned int index = 0; index < nstars(); index++) {
		try {
			Tycho2Star	star = find(index);
			if ((window.contains(star))
				&& (magrange.contains(star.mag()))) {
				result->insert(star);
			}
		} catch (std::exception& x) {
/*
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot get star %u: %s",
				index, x.what());
*/
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %u stars", result->size());
	return resultptr;
}

} // namespace catalog
} // namespace astro
