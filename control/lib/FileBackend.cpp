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
				const MagnitudeRange& magrange) {
	// prepare the result set
	Catalog::starset	*result = new Catalog::starset;
	Catalog::starsetptr	resultptr(result);

#define	Hipparcos_Tycho2_Cutover	7.
#define	Tycho2_Ucac4_Cutover	10.

	// find the range of magnitudes we want to get from the Hipparcos
	// catalog
	MagnitudeRange	hiprange = magrange;
	MagnitudeRange	tycho2range = magrange;
	MagnitudeRange	ucac4range = magrange;
	if (magrange.contains(Hipparcos_Tycho2_Cutover)) {
		hiprange.faintest() = Hipparcos_Tycho2_Cutover;
		tycho2range.brightest() = Hipparcos_Tycho2_Cutover;
	}
	if (magrange.contains(Tycho2_Ucac4_Cutover)) {
		tycho2range.faintest() = Tycho2_Ucac4_Cutover;
		ucac4range.brightest() = Tycho2_Ucac4_Cutover;
	}

	// handle cases that need only part of the catalogs
	if (magrange.faintest() < Hipparcos_Tycho2_Cutover) {
		tycho2range = MagnitudeRange(0, 0);
		ucac4range = MagnitudeRange(0, 0);
	}
	if (magrange.brightest() > Hipparcos_Tycho2_Cutover) {
		hiprange = MagnitudeRange(0, 0);
	}
	if (magrange.brightest() > Tycho2_Ucac4_Cutover) {
		hiprange = MagnitudeRange(0, 0);
		tycho2range = MagnitudeRange(0, 0);
	}
	if (magrange.faintest() < Tycho2_Ucac4_Cutover) {
		ucac4range = MagnitudeRange(0, 0);
	}

	// if any there are stars requested from the Hipparcos catalog, get them
	if (!hiprange.empty()) {
		// get brightest stars from Hipparcos catalog
		Hipparcos	catalog(hipparcosfile);
		Hipparcos::starsetptr	stars = catalog.find(window, hiprange);
		Hipparcos::starset::const_iterator	s;
		for (s = stars->begin(); s != stars->end(); s++) {
			Star	star = *s;
			result->insert(star);
		}
	}

	// get the intermediate stars from the Tycho2 catalog, but skip the
	// stars already retrieved from the Hipparcos catalog
	if (!tycho2range.empty()) {
		Tycho2	catalog(tycho2file);
		Tycho2::starsetptr	stars = catalog.find(window, magrange);
		Tycho2::starset::const_iterator	s;
		for (s = stars->begin(); s != stars->end(); s++) {
			// only take stars not in the Hipparcos catalog and
			// brighter than magnitude 10, as we will get the
			// fainter stars from Ucac4
			if ((!s->isHipparcosStar()) && (s->mag() < 10)) {
				Star	star = *s;
				result->insert(star);
			}
		}
	}

	// get all matching stars from the UCAC4 catalog
	if (!ucac4range.empty()) {
		Ucac4	catalog(ucac4dir);
		Ucac4::starsetptr	stars = catalog.find(window, magrange);
		Ucac4::starset::const_iterator	s;
		for (s = stars->begin(); s != stars->end(); s++) {
			// only take stars fainter than mag 10.0, as the
			// brighter stars already came from Tycho2.
			// There is a probability < 0.001 to loose a star due
			// to the cutoff
			if (s->mag() > 9.99) {
				Star	star = *s;
				result->insert(star);
			}
		}
	}

	return resultptr;
}

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
