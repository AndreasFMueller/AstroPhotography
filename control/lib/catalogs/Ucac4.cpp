/*
 * Ucac4.cpp -- Ucac4 catalog access implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Ucac4.h"
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <includes.h>

namespace astro {
namespace catalog {

std::string	Ucac4::zonefilename(uint16_t zone) const {
	std::string	filename = stringprintf("%s/u4b/z%03hu",
		_directory.c_str(), zone);
	return filename;
}

std::string	Ucac4::indexfilename() const {
	std::string	filename = stringprintf("%s/u4i/u4index.asc",
		_directory.c_str());
	return filename;
}

static void	checkfile(const std::string& filename) {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "checking file %s", filename.c_str());
	struct stat	sb;
	if (stat(filename.c_str(), &sb) < 0) {
		std::string	msg = stringprintf("cannot stat %s: %s",
			filename.c_str(), strerror(errno));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}

Ucac4::Ucac4(const std::string& directory) : _directory(directory) {
	// check existence of index file
	checkfile(indexfilename());

	// check existence of all zone files
	for (uint16_t zone = 1; zone <= 900; zone++) {
		checkfile(zonefilename(zone));
	}

	// make sure the zone is empty
	cachedzone = NULL;
}

Ucac4::~Ucac4() {
}

unsigned long	Ucac4::numberOfStars() {
	unsigned long	result = 0;
	for (uint16_t zone = 1; zone <= 900; zone++) {
		result += getzone(zone)->numberOfStars();
	}
	return result;
}

Ucac4ZonePtr	Ucac4::zone(uint16_t zone) const {
	std::string	zfn = zonefilename(zone);
	return Ucac4ZonePtr(new Ucac4Zone(zone, zfn));
}

/**
 * \brief Get a particular zone in the cache
 */
Ucac4ZonePtr	Ucac4::getzone(uint16_t z) {
	if ((cachedzone == NULL) || (cachedzone->zone() != z)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "opening zone %hd", z);
		cachedzone = zone(z);
	} 
	return cachedzone;
}

/**
 * \brief find a particular star
 */
Ucac4Star	Ucac4::find(const RaDec& /* position */) {
	// XXX imlementation missing
	Ucac4Star	result("unknown");
	return result;
}

/**
 * \brief find a star based on the UCAC4 number
 */
Star	Ucac4::find(const std::string& ucacnumber) {
	return find(Ucac4StarNumber(ucacnumber));
}

/**
 * \brief find a star based on the parsd UCAC4 number
 */
Ucac4Star	Ucac4::find(const Ucac4StarNumber& number) {
	return find(number.zone(), number.number());
}

/**
 * \brief Find a star based on zone and number
 */
Ucac4Star	Ucac4::find(uint16_t zone, uint32_t number) {
#if 0
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"getting star from zone %hu, number %u",
		zone, number);
#endif

	// open the zone file
	return getzone(zone)->get(number);
}

/**
 * \brief Get the interval of zone numbers touched by a window
 */
std::pair<uint16_t, uint16_t>	Ucac4::zoneinterval(const SkyWindow& window) {
	std::pair<double, double>	interval = window.decinterval();
	uint16_t	minzone = 1 + floor((interval.first + M_PI / 2) / (0.2 * M_PI / 180));
	if (minzone < 0) { minzone = 0; }
	uint16_t	maxzone = 1 + floor((interval.second + M_PI / 2) / (0.2 * M_PI / 180));
	if (maxzone > 900) { maxzone = 900; }
	return std::make_pair(minzone, maxzone);
}

/**
 * \brief Find out whether a zone touches a window
 */
bool	Ucac4::touches(uint16_t zone, const SkyWindow& window) {
	std::pair<uint16_t, uint16_t>	interval = zoneinterval(window);
	return ((interval.first <= zone) && (zone <= interval.second));
}

/**
 * \brief Retrieve all stars in a window
 */
Catalog::starsetptr	Ucac4::find(const SkyWindow& window,
				const MagnitudeRange& magrange) {
	// prepare the empty result set
	Ucac4Zone::starsetptr	resultptr(new Ucac4Zone::starset());

	// find minimum an maximum zone numbers
	std::pair<uint16_t, uint16_t>	interval = zoneinterval(window);
	uint16_t	minzone = interval.first;
	uint16_t	maxzone = interval.second;

	// iterate through zone numbers
	uint32_t	counter = 0;
	for (uint16_t zoneno = minzone; zoneno <= maxzone; zoneno++) {
		Ucac4ZonePtr	z = zone(zoneno);
		z->add(resultptr, window, magrange);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%u stars found, %u in set", counter,
		resultptr->size());

	// now copy everything into a set of normal stars
	Catalog::starsetptr	starresult
		= Catalog::starsetptr(new Catalog::starset());
	Ucac4Zone::starset::const_iterator	i;
	for (i = resultptr->begin(); i != resultptr->end(); i++) {
		starresult->insert(*i);
	}

	// thats it
	return starresult;
}

CatalogIterator	Ucac4::findIter(const SkyWindow& window,
	const MagnitudeRange& magrange) {
	IteratorImplementationPtr	impl(
		new Ucac4WindowIterator(*this, window, magrange));
	return CatalogIterator(impl);
}

CatalogIterator	Ucac4::begin() {
	IteratorImplementationPtr	impl(new Ucac4Iterator(1, 0, *this));
	return CatalogIterator(impl);
}

CatalogIterator	Ucac4::end() {
	IteratorImplementationPtr	impl(new Ucac4Iterator(901, 0, *this));
	return CatalogIterator(impl);
}

} // namespace catalog
} // namespace astro
