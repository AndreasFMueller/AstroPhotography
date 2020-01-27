/*
 * SAO.cpp -- SAO catalog access implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "SAO.h"
#include <AstroDebug.h>
#include <includes.h>
#include <AstroFormat.h>

namespace astro {
namespace catalog {

static std::string	sao_filename(const std::string filename) {
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
		f = filename + std::string("/sao.dat");
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
 * \brief Create a SAO catalog instance
 *
 * \param filename	the SAO main database file, usually called
 *			sao.dat
 */
SAO::SAO(const std::string& filename)
	: MappedFile(sao_filename(filename), 205) {
	backendname = stringprintf("SAO(%s)", filename.c_str());
	int	skipped = 0;
	for (size_t recno = 0; recno < nrecords(); recno++) {
		std::string	record = get(recno);
		try {
			SAOStar	star(record);
			stars.insert(std::make_pair(star.sao, star));
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "record %d skipped: %s",
				recno, x.what());
			skipped++;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d records skipped", skipped);
}

/**
 * \brief Destructor
 */
SAO::~SAO() {
}

/**
 * \brief Retrieve a star using the SAO number
 */
SAOStar	SAO::find(unsigned int sao) const {
	starmap_t::const_iterator	s = stars.find(sao);
	if (s == stars.end()) {
		throw std::runtime_error("illegal sao number");
	}
	return s->second;
}

/**
 * \brief Retrieve a star based on the SAO name
 */
Star	SAO::find(const std::string& name) {
	return find(std::stoi(name.substr(3)));
}

/**
 * \brief Retrieve stars in a a window an not too faint
 */
SAO::starsetptr	SAO::find(const SkyWindow& window,
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d SAO stars",
		resultptr->size());
	return resultptr;
}

/**
 * \brief Get the number of stars in the catalog
 */
unsigned long	SAO::numberOfStars() {
	return stars.size();
}

CatalogIterator	SAO::begin() {
	IteratorImplementationPtr impl(new SAOIterator(stars));
	return CatalogIterator(impl);
}

} // namespace catalog
} // namespace astro
