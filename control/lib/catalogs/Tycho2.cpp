/*
 * Tycho2.cpp -- Tycho2 star catalog class declarations
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Tycho2.h"
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
	if (line.size() != TYCHO2_RECORD_LENGTH) {
		std::string	msg = stringprintf("bad record length %d",
					line.size());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// set the catalog information
	catalog('T');
	std::string	number = line.substr(0,4) + line.substr(5, 5)
					+ line.substr(11, 1);
	catalognumber(std::stoull(number));

	// get magnitude
	float	vt = std::stod(line.substr(123, 6));
	float	bt = vt;
	try {
		bt = std::stod(line.substr(110, 6));
	} catch (...) { }
	mag(vt - 0.090 * (bt - vt));

	// RA and DEC
	try {
		ra().degrees(std::stod(line.substr(15, 12)));
		dec().degrees(std::stod(line.substr(28, 12)));
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("cannot parse position: %s",
					x.what());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// proper motion
	try {
		pm().ra().degrees(std::stod(line.substr(41, 7)) / 3600000);
		pm().dec().degrees(std::stod(line.substr(49, 7)) / 3600000);
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("cannot parse proper motion:"
					" %s", x.what());
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

Tycho2Star::Tycho2Star(const std::string& line)
	: Star(std::string("T") + line.substr(0, 12)) {
	setup(line);
}

//////////////////////////////////////////////////////////////////////
// Tycho2 catalog class implementation
//////////////////////////////////////////////////////////////////////
static std::string	Tycho2filename(const std::string& filename) {
	// is filename accessible?
	struct stat	sb;
	if (stat(filename.c_str(), &sb) < 0) {
		std::string	msg = stringprintf("cannot access '%s': %s",
			filename.c_str(), strerror(errno));
		throw std::runtime_error(msg);
	}

	// if the name is a directory, we append the standard filename
	std::string	f(filename);
	if (sb.st_mode & S_IFDIR) {
		f = filename + std::string("/tyc2.dat");
		if (stat(f.c_str(), &sb) < 0) {
			std::string	msg = stringprintf("cannot access '%s':"
				" %s", f.c_str(), strerror(errno));
			throw std::runtime_error(msg);
		}
	}

	// the file name should now point to a regular file
	if (!(sb.st_mode & S_IFREG)) {
		std::string	msg = stringprintf("'%s' is not a regular file",
			f.c_str());
		throw std::runtime_error(msg);
	}

	// if we get to this point, then f is the correct tycho2 data file name
	return f;
}

Tycho2::Tycho2(const std::string& filename)
	: MappedFile(filename, TYCHO2_RECORD_LENGTH), _filename(filename) {
}

Tycho2::~Tycho2() {
}

/**
 * \brief get a star from the catalog
 */
Tycho2Star	Tycho2::find(unsigned int index) const {
	if (index >= nstars()) {
		throw std::runtime_error("not that many stars in Tycho2");
	}
	return Tycho2Star(get(index));
}

/**
 * \brief Retrieve a star based on the name
 */
Star	Tycho2::find(const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve star '%s'", name.c_str());
	return find(index(name));
}

/**
 * \brief Get the key for a given line
 */
std::string	Tycho2::key(int index) const {
	return get(index).substr(0, 12);
}

/**
 * \brief find the index based on a name
 */
int	Tycho2::index(const std::string& name) {
	if (name[0] != 'T') {
		std::string	msg = stringprintf("'%s' is not a Tycho2 name",
			name.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	std::string	key = name.substr(1);
	int	min = 0;
	int	max = nstars() - 1;
	std::string	minkey = Tycho2::key(min);
	std::string	maxkey = Tycho2::key(max);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"looking for '%s' between record %d and %d, '%s' and '%s'",
		key.c_str(), min, max, minkey.c_str(), maxkey.c_str());

	while (min < max) {
		if (minkey == key) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "found %s",
				minkey.c_str());
			return min;
		}
		if (maxkey == key) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "found %s",
				maxkey.c_str());
			return max;
		}
		int	current = (min + max) / 2;
		std::string	currentkey = Tycho2::key(current);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "current = %d, currentkey = %s",
			current, currentkey.c_str());
		if (currentkey <= key) {
			min = current;
			minkey = currentkey;
		} else {
			max = current;
			maxkey = currentkey;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "[%d, %d] - [%s, %s]",
			min, max, minkey.c_str(), maxkey.c_str());
	}

	return min;
}

/**
 * \brief get all stars from a window
 */
Tycho2::starsetptr	Tycho2::find(const SkyWindow& window,
				const MagnitudeRange& magrange) {
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

/**
 * \brief Get the number of stars in the Tycho2 catalog
 */
unsigned long	Tycho2::numberOfStars() {
	return nrecords();
}

CatalogIterator	Tycho2::begin() {
	IteratorImplementationPtr	impl(new Tycho2Iterator(0, *this));
	return CatalogIterator(impl);
}

CatalogIterator	Tycho2::end() {
	IteratorImplementationPtr	impl(new Tycho2Iterator(numberOfStars(),
						*this));
	return CatalogIterator(impl);
}

//////////////////////////////////////////////////////////////////////
// Tycho2 iterator implementation
//////////////////////////////////////////////////////////////////////

Tycho2Iterator::Tycho2Iterator(unsigned int index, Tycho2& catalog)
	: _index(index), _catalog(catalog) {
}

Tycho2Iterator::~Tycho2Iterator() {
}

Star 	Tycho2Iterator::operator*() {
	return _catalog.find(_index);
}

bool	Tycho2Iterator::operator==(const Tycho2Iterator& other) const {
	return (_index == other._index);
}

bool	Tycho2Iterator::operator==(const IteratorImplementation& other) const {
	return equal_implementation(this, other);
}

void	Tycho2Iterator::increment() {
	++_index;
	if (_index > _catalog.numberOfStars()) {
		_index = _catalog.numberOfStars();
	}
}

std::string	Tycho2Iterator::toString() const {
	return stringprintf("%d", _index);
}

} // namespace catalog
} // namespace astro
