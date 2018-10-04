/*
 * BSCStar.cpp -- bright star catalog star implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "BSC.h"
#include <fstream>
#include <AstroDebug.h>
#include <includes.h>
#include <AstroFormat.h>
#include <typeinfo>

namespace astro {
namespace catalog {

static std::string	BSCname(int number) {
	return stringprintf("BSC%04d", number);
}

static std::string	BSCname(const char *line) {
	int	number = std::stoi(std::string(line, 0, 4));
	return BSCname(number);
}

BSCStar::BSCStar(const char *l) : Star(BSCname(l)) {
	catalog('B');
	std::string	line(l);
	try {
		number = std::stoi(line.substr(0, 4));
		catalognumber(number);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot parse number: %s",
			line.substr(0, 4).c_str());
		throw std::runtime_error("cannot parse object number");
	}

	try {
		longname(trim(line.substr(5, 9)));
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot extract name: %s",
			line.substr(4, 10).c_str());
		throw std::runtime_error("cannot set long name");
	}

	try {
		sao = std::stoi(line.substr(31,6));
	} catch (...) {
		sao = 0;
	}
	try {
		mag(std::stod(line.substr(102, 5)));
	} catch (...) {
		std::string	msg = std::string("cannot parse magnitude: '")
			+ line.substr(102, 5) + std::string("'");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// RA
	try {
		ra().hours(std::stod(line.substr(75, 2))
			+ std::stod(line.substr(77, 2)) / 60
			+ std::stod(line.substr(79, 4)) / 3600);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot parse RA");
		throw std::runtime_error("cannot parse right ascension");
	}

	// DEC
	try {
		dec().degrees(
			((line.substr(83, 1) == "-") ? -1 : 1) * 
				(std::stod(line.substr(84, 2))
				 + std::stod(line.substr(86, 2)) / 60
				 + std::stod(line.substr(88, 2)) / 3600));
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot parse DEC");
		throw std::runtime_error("cannot parse declination");
	}

	// proper motion
	try {
		pm().ra().degrees(std::stod(line.substr(148, 6)) / 3600);
		pm().dec().degrees(std::stod(line.substr(154, 6)) / 3600);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot parse proper motion");
	}
};

bool	BSCStar::operator<(const BSCStar& other) const {
	return number < other.number;
}

bool	BSCStar::operator>(const BSCStar& other) const {
	return number > other.number;
}

bool	BSCStar::operator<=(const BSCStar& other) const {
	return number <= other.number;
}

bool	BSCStar::operator>=(const BSCStar& other) const {
	return number >= other.number;
}

} // namespace catalog
} // namespace astro
