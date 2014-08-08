/*
 * Catalog.cpp -- Catalog access implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <includes.h>
#include <CatalogBackend.h>

namespace astro {
namespace catalog {

//////////////////////////////////////////////////////////////////////
// Celestial Object implementation
//////////////////////////////////////////////////////////////////////
/**
 * \brief Compute proper motion corrected position of an object
 */
RaDec	CelestialObject::position(const double epoch) const {
	RaDec	result;
	result.ra() = ra() + pm().ra() * epoch;
	result.dec() = dec() + pm().dec() * epoch;
	return result;
}

//////////////////////////////////////////////////////////////////////
// Star implementation
//////////////////////////////////////////////////////////////////////
/**
 * \brief string representation of a star
 */
std::string	Star::toString() const {
	return stringprintf("%s %s %.2f",
			ra().hms().c_str(), dec().dms().c_str(), mag());
}

//////////////////////////////////////////////////////////////////////
// DeepSkyObject implementation
//////////////////////////////////////////////////////////////////////
/**
 * \brief string representation of a DeepSkyObject
 */
std::string	DeepSkyObject::toString() const {
	return stringprintf("%s: %s %s %.2f (%s)", name.c_str(),
			ra().hms().c_str(), dec().dms().c_str(), mag(),
			constellation.c_str());
}

//////////////////////////////////////////////////////////////////////
// MagnitudeRange implementation
//////////////////////////////////////////////////////////////////////
std::string	MagnitudeRange::toString() const {
	return stringprintf("[%.2f, %.2f]", brightest(), faintest());
}


//////////////////////////////////////////////////////////////////////
// Catalog implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Create a compilied catalog
 *
 * No catalog is complete enough for our purposes. But getting stars from
 * the files can be quite time consuming. This problem is solved by the
 * database backend, bat that requires quite a bit of disk space.
 * To unify the catalog access and make it transparent for the application,
 * the Catalog constructor decides about the backend to use based on the
 * type of the file argument. If the filename refers to a directory, it
 * is assumed that the file based backend should be used. If filename
 * names a file, it is assumed that this is a database file, and the
 * database backend is used on this file.
 * \param filename	name of database file or base directory for catalogs
 */
Catalog::Catalog(const std::string& filename) {
	// find out whether filename points to a file (i.e. use database
	// backend) or to a directory (use file backend)
	struct stat	sb;
	if (stat(filename.c_str(), &sb) < 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot stat %s: %s",
			filename.c_str(), strerror(errno));
		throw std::runtime_error("cannot stat catalog file");
	}

	// depending on the file type, open different backends
	if (sb.st_mode & S_IFDIR) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "open file backend based at %s",
			filename.c_str());
		backend = CatalogBackendPtr(new FileBackend(filename));
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "open db backend on file %s",
			filename.c_str());
		backend = CatalogBackendPtr(new DatabaseBackend(filename));
	}
}

/**
 * \brief Retrieve stars from a compiled catalog
 */
Catalog::starsetptr	Catalog::find(const SkyWindow& window,
				const MagnitudeRange& magrange) {
	return backend->find(window, magrange);
}

/**
 * \brief Retrieve stars from a compiled catalog
 */
Star	Catalog::find(const std::string& name) {
	return backend->find(name);
}

} // namespace catalog
} // namespace astro
