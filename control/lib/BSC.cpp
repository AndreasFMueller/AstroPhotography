/*
 * BSC.cpp -- bright star catalog implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <BSC.h>
#include <fstream>
#include <AstroDebug.h>
#include <includes.h>

namespace astro {
namespace catalog {

//////////////////////////////////////////////////////////////////////
// BSCStar implementation
//////////////////////////////////////////////////////////////////////

BSCStar::BSCStar(const char *l) {
	std::string	line(l);
	number = std::stoi(line.substr(0, 4));
	name = line.substr(4, 10);
	try {
		sao = std::stoi(line.substr(31,6));
	} catch (...) {
		sao = 0;
	}
	_mag = std::stod(line.substr(102, 5));
	// RA
	ra().hours(std::stod(line.substr(75, 2))
		+ std::stod(line.substr(77, 2)) / 60
		+ std::stod(line.substr(79, 4)) / 3600);

	// DEC
	dec().degrees(
		((line.substr(83, 1) == "-") ? -1 : 1) * 
			(std::stod(line.substr(84, 2))
			 + std::stod(line.substr(86, 2)) / 60
			 + std::stod(line.substr(88, 2)) / 3600));

	// proper motion
	pm().ra().degrees(std::stod(line.substr(148, 6)) / 3600);
	pm().dec().degrees(std::stod(line.substr(154, 6)) / 3600);
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

std::string	BSCStar::toString() const {
	return Star::toString() + " " + name;
}

//////////////////////////////////////////////////////////////////////
// BSC implementation
//////////////////////////////////////////////////////////////////////
BSC::BSC(const std::string& filename, const std::string& notesfile) {
	// open the catalog file
	std::ifstream	in(filename);

	// read all lines from the file
	do {
		char	line[200];
		in.getline(line, 200);
		if (!in.eof()) {
			try {
				BSCStar	star(line);
				stars.insert(std::make_pair(star.number, star));
			} catch (std::exception& x) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"object '%s' skipped: %s",
					line, strerror(errno));
			}
		}
	} while (!in.eof());

	// close the input file
	in.close();

	// open the notes file
	std::ifstream	notes(notesfile);

	// read a line from the notes file and append it to the appropriate
	// star
	do {
		char	l[135];
		notes.getline(l, 135);
		if (!notes.eof()) {
			std::string	line(l);
			//debug(LOG_DEBUG, DEBUG_LOG, 0, "notes from '%s'", l);
			unsigned short	number = std::stoi(line.substr(0, 5));
			std::string	value = line.substr(6);
			if (stars.end() != stars.find(number)) {
				stars.find(number)->second.notes.push_back(value);
			}
		}
	} while (!notes.eof());

	// close the notes file
	notes.close();
}

/**
 * \brief retrieve an individual star
 */
const BSCStar&	BSC::find(int number) const {
	if (stars.find(number) == stars.end()) {
		throw std::runtime_error("star number out of range");
	}
	return stars.find(number)->second;
}

/**
 * \brief Get a list of stars of a given minimum magnitude
 */
BSC::starset	BSC::find(const SkyWindow& window,
				const MagnitudeRange& magrange) const {
	starset	result;
	std::map<unsigned short, BSCStar>::const_iterator	i;
	for (i = stars.begin(); i != stars.end(); i++) {
		if ((magrange.contains(i->second.mag()))
			&& window.contains(i->second)) {
			result.insert(i->second);
		}
	}
	return result;
}

} // namespace catalog
} // namespace astro
