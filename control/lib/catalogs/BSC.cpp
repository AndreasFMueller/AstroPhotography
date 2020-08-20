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
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	if (stat(_notesfile.c_str(), &sb) < 0) {
		std::string	msg = stringprintf("cannot stat %s: %s",
			_notesfile.c_str(), strerror(errno));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	backendname = stringprintf("BSC(%s)", _filename.c_str());

	// open the catalog file
	std::ifstream	in(_filename);
	if (in.fail()) {
		std::string	msg = stringprintf("cannot open %s",
			_filename.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s",msg.c_str());
		throw std::runtime_error(msg);
	}

	// read all lines from the file
	int	rejected = 0;
	do {
		char	line[200];
		in.getline(line, 200);
		if (!in.eof()) {
			try {
				BSCStar	star(line);
				stars.insert(std::make_pair(star.number, star));
			} catch (const std::exception& x) {
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
	if (notes.fail()) {
		std::string	msg = stringprintf("notes file '%s' missing",
			_notesfile.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

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
	IteratorImplementationPtr	impl(new BSCIterator(stars));
	return CatalogIterator(impl);
}

/**
 * \brief Find a set of stars
 *
 * \param name		prefix of star names
 * \param maxstars	maximum number of stars to retrieve
 */
Catalog::starsetptr	BSC::findLike(const std::string& name,
		size_t maxstars) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get stars for prefix %s", name.c_str());
	Catalog::starsetptr	result(new Catalog::starset());
	// ignore short names
	if (name.size() < 3) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "short name");
		throw std::runtime_error("short name");
	}
	if (name.substr(0,3) != std::string("BSC")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not a BSC name");
		throw std::runtime_error("not a BSC name");
	}

	// find the first number
	size_t	firstnumber = std::stoi((name + std::string("0000")).substr(3, 4));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "firstnumber = %d", firstnumber);

	// find the last matching number
	int	intervallength = 1;
	for (int i = name.size(); i < 7; i++) {
		intervallength *= 10;
	}
	size_t	lastnumber = firstnumber + intervallength;

	// scan for these numbers
	size_t	i = firstnumber;
	while ((result->size() < maxstars) && (i < lastnumber)) {
		try {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "get BSC %d", i);
			Star	star = find(i);
			result->insert(star);
		} catch (const std::exception& x) {
		}
		i++;
	}

	return result;
}

} // namespace catalog
} // namespace astro
