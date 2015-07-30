/*
 * BSC.cpp -- bright star catalog implementation
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

//////////////////////////////////////////////////////////////////////
// BSCStar implementation
//////////////////////////////////////////////////////////////////////

static std::string	BSCname(int number) {
	return stringprintf("BSC%04d", number);
}

static std::string	BSCname(const char *line) {
	int	number = std::stoi(std::string(line, 0, 4));
	return BSCname(number);
}

BSCStar::BSCStar(const char *l) : Star(BSCname(l)) {
	std::string	line(l);
	try {
		number = std::stoi(line.substr(0, 4));
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot parse number: %s",
			line.substr(0, 4).c_str());
		throw std::runtime_error("cannot parse object number");
	}
	try {
		longname(rtrim(line.substr(5, 9)));
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

//////////////////////////////////////////////////////////////////////
// BSC implementation
//////////////////////////////////////////////////////////////////////

BSC::BSC(const std::string& filename, const std::string& notesfile)
	: _filename(filename), _notesfile(notesfile) {
	setup();
}

BSC::BSC(const std::string& basedir)
	: _filename(basedir + "/catalog"), _notesfile(basedir + "/notes") {
	setup();
}

BSC::~BSC() {
}

void	BSC::setup() {
	// check whether we have the files
	struct stat	sb;
	if (stat(_filename.c_str(), &sb) < 0) {
		std::string	msg = stringprintf("cannot stat %s: %s",
			_filename.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	if (stat(_notesfile.c_str(), &sb) < 0) {
		std::string	msg = stringprintf("cannot stat %s: %s",
			_notesfile.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// open the catalog file
	std::ifstream	in(_filename);

	// read all lines from the file
	int	rejected = 0;
	do {
		char	line[200];
		in.getline(line, 200);
		if (!in.eof()) {
			try {
				BSCStar	star(line);
				stars.insert(std::make_pair(star.number, star));
			} catch (std::exception& x) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"object '%4.4s' skipped: %s",
					line, x.what());
				rejected++;
			}
		}
	} while (!in.eof());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d stars rejected, %d stars in catalog",
		rejected, stars.size());

	// close the input file
	in.close();

	// open the notes file
	std::ifstream	notes(_notesfile);

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
BSCStar	BSC::find(int number) {
	if (stars.find(number) == stars.end()) {
		throw std::runtime_error("star number out of range");
	}
	return stars.find(number)->second;
}

/**
 * \brief Get a star by name
 */
Star	BSC::find(const std::string& name) {
	return find(std::stoi(name.substr(3)));
}

/**
 * \brief Get a list of stars of a given minimum magnitude
 */
Catalog::starsetptr	BSC::find(const SkyWindow& window,
				const MagnitudeRange& magrange) {
	starset	*result = new Catalog::starset();
	starsetptr	resultptr(result);
	starmap_t::const_iterator	i;
	for (i = stars.begin(); i != stars.end(); i++) {
		if ((magrange.contains(i->second.mag()))
			&& window.contains(i->second)) {
			result->insert(i->second);
		}
	}
	return resultptr;
}

/**
 * \brief Get the number of stars in the BSC catalog
 */
unsigned long	BSC::numberOfStars() {
	return stars.size();
}

CatalogIterator	BSC::begin() {
	IteratorImplementationPtr	impl(new BSCIterator(stars.begin()));
	return CatalogIterator(impl);
}

CatalogIterator	BSC::end() {
	IteratorImplementationPtr	impl(new BSCIterator(stars.end()));
	return CatalogIterator(impl);
}

//////////////////////////////////////////////////////////////////////
// BSCIterator implemenation
//////////////////////////////////////////////////////////////////////
BSCIterator::BSCIterator(BSC::starmap_t::iterator i) : _i(i) {
}

Star	BSCIterator::operator*() {
	return _i->second;
}

bool	BSCIterator::operator==(const BSCIterator& other) const {
	return (_i == other._i);
}

bool	BSCIterator::operator==(const IteratorImplementation& other) const {
	return equal_implementation(this, other);
}

void	BSCIterator::increment() {
	++_i;
}

std::string	BSCIterator::toString() const {
	return stringprintf("%d", _i->first);
}

} // namespace catalog
} // namespace astro
