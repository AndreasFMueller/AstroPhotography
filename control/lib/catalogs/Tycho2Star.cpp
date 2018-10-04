/*
 * Tycho2Star.cpp -- Tycho2 star implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Tycho2.h"
#include <includes.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <limits>

namespace astro {
namespace catalog {

#define TYCHO2_RECORD_LENGTH	207

void	Tycho2Star::setup(unsigned int index, const std::string& line) {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "creating star from line '%s'",
	//	line.c_str());

	// check position flag
	if ('X' == line[13]) {
		std::string	msg = stringprintf("record %u, no position",
					index);
		throw std::runtime_error(msg);
	}
	if (line.size() != TYCHO2_RECORD_LENGTH) {
		std::string	msg = stringprintf("bad record[%u] length %d",
					index, line.size());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// set the catalog information
	catalog('T');
	std::string	number = line.substr(0,4) + line.substr(5, 5)
					+ line.substr(11, 1);
	catalognumber(std::stoull(number));

	// get magnitude
	float	vt = std::numeric_limits<float>::infinity();
	try {
		vt = std::stod(line.substr(123, 6));
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("cannot parse magnitude: %s",
			line.substr(123, 6).c_str());
		throw std::runtime_error(msg);
	}
	float	bt = vt;
	try {
		bt = std::stod(line.substr(110, 6));
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("cannot parse magnitude: %s",
			line.substr(110, 6).c_str());
		throw std::runtime_error(msg);
	}
	mag(vt - 0.090 * (bt - vt));

	// RA and DEC
	try {
		ra().degrees(std::stod(line.substr(15, 12)));
		dec().degrees(std::stod(line.substr(28, 12)));
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("record[%u] cannot parse "
					"position: %s", index, x.what());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// proper motion
	try {
		pm().ra().degrees(std::stod(line.substr(41, 7)) / 3600000);
		pm().dec().degrees(std::stod(line.substr(49, 7)) / 3600000);
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("cannot parse proper motion "
					"in record[%u]: %s", index, x.what());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// get the hipparcos number, if this is a hipparcos star
	std::string	hipnumber = ltrim(line.substr(142, 6));
	if (hipnumber.size() > 0) {
		std::string	hipname = stringprintf("HIP%06u",
						std::stoi(hipnumber));
		setDuplicate('H', hipname);
	}
}

Tycho2Star::Tycho2Star(const std::string& line, unsigned int index)
	: Star(std::string("T") + line.substr(0, 12)) {
	setup(index, line);
}

} // namespace catalog
} // namespace astro
