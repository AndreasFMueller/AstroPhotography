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
	: MappedFile(hipparcos_filename(filename), 451),
	  precession(2000.0 - 1991.25) {
	backendname = stringprintf("Hipparcos(%s)", filename.c_str());
	int	skipped = 0;
	for (size_t recno = 0; recno < nrecords(); recno++) {
		std::string	record = get(recno);
		try {
			HipparcosStar	star(record);
			// XXX do we really need to precess stars?
			//star.precess(precession);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrive stars in range %s",
		magrange.toString().c_str());
	starset	*result = new starset();
	starsetptr	resultptr(result);
	starmap_t::const_iterator	s;
	for (s = stars.begin(); s != stars.end(); s++) {
		if (window.contains(s->second)
			&& (magrange.contains(s->second.mag()))) {
			result->insert(s->second);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d Hipparcos stars",
		resultptr->size());
	return resultptr;
}

/**
 * \brief Get the number of stars in the catalog
 */
unsigned long	Hipparcos::numberOfStars() {
	return stars.size();
}

CatalogIterator	Hipparcos::begin() {
	IteratorImplementationPtr impl(new HipparcosIterator(stars));
	return CatalogIterator(impl);
}

} // namespace catalog
} // namespace astro
