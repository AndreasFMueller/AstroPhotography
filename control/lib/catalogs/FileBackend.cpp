/*
 * FileBAckend.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil 
 */
#include "CatalogBackend.h"
#include <includes.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <Hipparcos.h>
#include "Tycho2.h"
#include "Ucac4.h"

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
 * \brief retrieve stars from all catalogs combined
 *
 * This method uses the Hipparcos, Tycho-2 and UCAC4 catalogs in that sequence
 * to get a complete catalog up to magnitude 16. It stops when it can be
 * sure that the catalog is complete. This will happen when the limiting
 * magnitude of the request (the magrange.faintest() value) is smaller than
 * the magnitude up to which the catalog in question is complete. For
 * magnitutes <= 7, the Hipparcos catalog is complete, for magnitudes <= 10,
 * the Tycho-2 catalog is complete with very high probability. 
 */
Catalog::starsetptr	FileBackend::find(const SkyWindow& window,
				const MagnitudeRange& magrange) {
	// prepare the result set
	Catalog::starset	*result = new Catalog::starset;
	Catalog::starsetptr	resultptr(result);

#define	Hipparcos_Complete_Magnitude	7.
#define	Tycho2_Complete_Magnitude	10.

	// if any there are stars requested from the Hipparcos catalog, get them
	{
		// get brightest stars from Hipparcos catalog
		Hipparcos	catalog(hipparcosfile);
		Hipparcos::starsetptr	stars = catalog.find(window, magrange);
		Hipparcos::starset::const_iterator	s;
		for (s = stars->begin(); s != stars->end(); s++) {
			Star	star = *s;
			result->insert(star);
		}
	}

	// if the faintest magnitude is brighter than the magnitude to
	// which the Hipparcos catalog is complete, then we can skip 
	// Tacho-2 and UCAC4
	if (magrange.faintest() < Hipparcos_Complete_Magnitude) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "faintest magnitude complete "
			"in Hipparcos catalog");
		return resultptr;
	}

	// get the intermediate stars from the Tycho2 catalog, but skip the
	// stars already retrieved from the Hipparcos catalog
	{
		Tycho2	catalog(tycho2file);
		Tycho2::starsetptr	stars = catalog.find(window, magrange);
		Tycho2::starset::const_iterator	s;
		for (s = stars->begin(); s != stars->end(); s++) {
			// only take stars not in the Hipparcos catalog and
			// brighter than magnitude 10, as we will get the
			// fainter stars from Ucac4
			if (!s->isHipparcosStar()) {
				Star	star = *s;
				result->insert(star);
			}
		}
	}

	// if the faintestmagnitude is bright enough so that the Tycho-2
	// catalog is complete up to this magnitude, then we can skip UCAC4
	if (magrange.faintest() < Tycho2_Complete_Magnitude) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "faintest magnitude complete "
			"in Tycho-2 catalog");
		return resultptr;
	}

	// get all matching stars from the UCAC4 catalog
	{
		Ucac4	catalog(ucac4dir);
		Ucac4::starsetptr	stars = catalog.find(window, magrange);
		Ucac4::starset::const_iterator	s;
		for (s = stars->begin(); s != stars->end(); s++) {
			if (!s->hiptyc2) {
				Star	star = *s;
				result->insert(star);
			}
		}
	}

	return resultptr;
}

/**
 * \brief Get a star from the unified catalogs by name
 */
Star	FileBackend::find(const std::string& name) {
	// check for Hipparcos stars
	if (name.substr(0, 3) == "HIP") {
		Hipparcos	catalog(hipparcosfile);
		HipparcosStar	hstar = catalog.find(std::stoi(name.substr(3)));
		Star	star = hstar;
		return star;
	}

	// check for Tycho2 star
	if (name.substr(0, 1) == "T") {
		Tycho2	catalog(tycho2file);
		Tycho2Star	tstar = catalog.find(std::stoi(name.substr(1)));
		Star	star = tstar;
		return star;
	}

	// check for Ucac4 star
	if (name.substr(0, 5) == "UCAC4") {
		Ucac4	catalog(ucac4dir);
		Ucac4Star	ustar = catalog.find(name);
		Star	star = ustar;
		return star;
	}
	throw std::runtime_error("unkonwn star name");
}

} // namespace catalog
} // namespace astro
