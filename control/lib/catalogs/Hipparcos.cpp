/*
 * Hipparcos.cpp -- Hipparcos catalog access implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Hipparcos.h"
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
HipparcosStar::HipparcosStar(const std::string& line)
	: Star(stringprintf("HIP%06u", std::stoi(std::string(line, 8, 6)))) {
	hip = std::stoi(std::string(line, 8, 6));
	catalog('H');
	catalognumber(hip);
	ra().hours(std::stoi(line.substr(17, 2))
		+ std::stoi(line.substr(20, 2)) / 60.
		+ std::stod(line.substr(23, 5)) / 3600.);
	dec().degrees(
		((line[29] == '-') ? (-1) : 1) *
		(std::stoi(line.substr(30, 2))
		 + std::stoi(line.substr(33, 2)) / 60.
		 + std::stod(line.substr(36, 4)) / 3600.));
	pm().ra().degrees(std::stod(line.substr(87, 8)) / 3600000.);
	pm().dec().degrees((std::stod(line.substr(96, 8)) / 3600000.)
		/ cos(dec()));
	mag(std::stod(line.substr(41, 5)));
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
static std::string	hipparcos_filename(const std::string filename) {
	// first find out whether filename is actually a directory name
	struct stat	sb;
	if (stat(filename.c_str(), &sb) < 0) {
		std::string	msg = stringprintf("cannot access '%s': %s",
			filename.c_str(), strerror(errno));
		throw std::runtime_error(msg);
	}

	// if the path is a directory, append the standard filename
	std::string	f(filename);
	if (sb.st_mode & S_IFDIR) {
		f = filename + std::string("/hip_main.dat");
		if (stat(f.c_str(), &sb) < 0) {
			std::string	msg = stringprintf("cannot access '%s':"
				" %s", f.c_str(), strerror(errno));
			throw std::runtime_error(msg);
		}
	}

	// at this point we should have an ordinary file
	if (!(sb.st_mode & S_IFREG)) {
		std::string	msg = stringprintf("'%s' is not a regular file",
			f.c_str());
		throw std::runtime_error(msg);
	}

	// if we get to this point, then f is the correct file name
	return f;
}

/**
 * \brief Create a Hipparcos catalog instance
 *
 * \param filename	the Hipparcos main database file, usually called
 *			hip_main.dat
 */
Hipparcos::Hipparcos(const std::string& filename)
	: MappedFile(hipparcos_filename(filename), 451) {
	int	skipped = 0;
	for (size_t recno = 0; recno < nrecords(); recno++) {
		std::string	record = get(recno);
		try {
			HipparcosStar	star(record);
			stars.insert(std::make_pair(star.hip, star));
		} catch (...) {
			skipped++;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d records skipped", skipped);
}

/**
 * \brief Destructor
 */
Hipparcos::~Hipparcos() {
}

/**
 * \brief Retrieve a star using the HIP number
 */
HipparcosStar	Hipparcos::find(unsigned int hip) const {
	starmap_t::const_iterator	s = stars.find(hip);
	if (s == stars.end()) {
		throw std::runtime_error("illegal hip number");
	}
	return s->second;
}

/**
 * \brief Retrieve a star based on the HIP name
 */
Star	Hipparcos::find(const std::string& name) {
	return find(std::stoi(name.substr(3)));
}

/**
 * \brief Retrieve stars in a a window an not too faint
 */
Hipparcos::starsetptr	Hipparcos::find(const SkyWindow& window,
				const MagnitudeRange& magrange) {
	starset	*result = new starset();
	starsetptr	resultptr(result);
	starmap_t::const_iterator	s;
	for (s = stars.begin(); s != stars.end(); s++) {
		if (window.contains(s->second)
			&& (magrange.contains(s->second.mag()))) {
			result->insert(s->second);
		}
	}
	return resultptr;
}

/**
 * \brief Get the number of stars in the catalog
 */
unsigned long	Hipparcos::numberOfStars() {
	return stars.size();
}

CatalogIterator	Hipparcos::begin() {
	IteratorImplementationPtr impl(new HipparcosIterator(stars.begin()));
	return CatalogIterator(impl);
}

CatalogIterator	Hipparcos::end() {
	IteratorImplementationPtr impl(new HipparcosIterator(stars.end()));
	return CatalogIterator(impl);
}

//////////////////////////////////////////////////////////////////////
// Hipparcos Iterator implementation
//////////////////////////////////////////////////////////////////////
HipparcosIterator::HipparcosIterator(Hipparcos::starmap_t::iterator i)
	: _i(i) {
}

Star	HipparcosIterator::operator*() {
	return _i->second;
}

bool	HipparcosIterator::operator==(const HipparcosIterator& other) const {
	return (_i == other._i);
}

bool	HipparcosIterator::operator==(const IteratorImplementation& other) const {
	return equal_implementation(this, other);
}

void	HipparcosIterator::increment() {
	++_i;
}

std::string	HipparcosIterator::toString() const {
	return stringprintf("%d", _i->first);
}

} // namespace catalog
} // namespace astro
