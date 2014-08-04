/*
 * FileBAckend.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil 
 */
#include <CatalogBackend.h>
#include <includes.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <Hipparcos.h>
#include <Tycho2.h>
#include <Ucac4.h>

namespace astro {
namespace catalog {

/**
 * \brief auxiliary function to check existence of files/directories
 */
static void	require(const std::string& filename) {
	struct stat	sb;
	if (stat(filename.c_str(), &sb) < 0) {
		std::string	msg = stringprintf("cannot stat '%s': %s",
			filename.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
	}
}

/**
 * \brief Create a file based catalog backend
 *
 * The constructor ensures that all the files or directories (in the case of
 * the Ucac4 catalog) are present
 */
FileBackend::FileBackend(const std::string& basedir) : _basedir(basedir) {
	// ensure that the files for the file backend exist
	hipparcosfile = _basedir + "/hipparcos/hip_main.dat";
	require(hipparcosfile);
	tycho2file = _basedir + "/tycho2/tyc2.dat";
	require(tycho2file);
	ucac4dir = _basedir + "/u4";
	require(ucac4dir);
}

/**
 * \brief destroy the backend
 */
FileBackend::~FileBackend() {
}

/**
 * \brief
 */
Catalog::starsetptr	FileBackend::find(const SkyWindow& window,
				double minimum_magnitude) {
	// prepare the result set
	Catalog::starset	*result = new Catalog::starset;
	Catalog::starsetptr	resultptr(result);
{
	// get brightest stars from Hipparcos catalog
	Hipparcos	catalog(hipparcosfile);
	std::set<HipparcosStar>	stars = catalog.find(window, minimum_magnitude);
	std::set<HipparcosStar>::const_iterator	s;
	for (s = stars.begin(); s != stars.end(); s++) {
		Star	star = *s;
		result->insert(star);
	}
}
	if (minimum_magnitude < 7) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"Tycho2 and UCAC4 catalog not needed");
		return resultptr;
	}

{
	// get the intermediate stars from the Tycho2 catalog, but skip the
	// stars already retrieved from the Hipparcos catalog
	Tycho2	catalog(tycho2file);
	std::set<Tycho2Star>	stars = catalog.find(window, minimum_magnitude);
	std::set<Tycho2Star>::const_iterator	s;
	for (s = stars.begin(); s != stars.end(); s++) {
		// only take stars not in the Hipparcos catalog and brighter
		// than magnitude 10, as we will get the fainter stars from
		// Ucac4
		if ((!s->isHipparcosStar()) && (s->mag() < 10)) {
			Star	star = *s;
			result->insert(star);
		}
	}
}

	if (minimum_magnitude < 10) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "UCAC4 catalog not needed");
		return resultptr;
	}

{
	// get all matching stars from the UCAC4 catalog
	Ucac4	catalog(ucac4dir);
	std::set<Ucac4Star>	stars = catalog.find(window, minimum_magnitude);
	std::set<Ucac4Star>::const_iterator	s;
	for (s = stars.begin(); s != stars.end(); s++) {
		// only take stars fainter than mag 10.0, as the brighter
		// stars already came from Tycho2. There is a probability
		// < 0.001 to loose a star due to the cutoff
		if (s->mag() > 9.99) {
			Star	star = *s;
			result->insert(star);
		}
	}
}

	return resultptr;
}

} // namespace catalog
} // namespace astro
