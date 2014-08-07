/*
 * Ucac4.cpp -- Ucac4 catalog access implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Ucac4.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <includes.h>

namespace astro {
namespace catalog {

//////////////////////////////////////////////////////////////////////
// Ucac4StarNumber
//////////////////////////////////////////////////////////////////////
Ucac4StarNumber::Ucac4StarNumber(const std::string& starnumber) {
	if (2 != sscanf(starnumber.c_str(), "UCAC4-%hu-%u",
		&_zone, &_number)) {
		std::string	msg = stringprintf("cannot parse UCAC4 star "
			"number '%s'", starnumber.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}

std::string	Ucac4StarNumber::toString() const {
	return stringprintf("UCAC4-%03hu-%06u", _zone, _number);
}

bool	Ucac4StarNumber::operator==(const Ucac4StarNumber& other) const {
	return (_zone == other._zone) && (_number == other._number);
}

bool	Ucac4StarNumber::operator!=(const Ucac4StarNumber& other) const {
	return !(*this == other);
}

bool	Ucac4StarNumber::operator<(const Ucac4StarNumber& other) const {
	if (_zone < other._zone) {
		return true;
	}
	if (_zone > other._zone) {
		return false;
	}
	return (_number < other._number);
}

std::ostream&	operator<<(std::ostream& out, const Ucac4StarNumber& star) {
	out << star.toString();
	return out;
}

//////////////////////////////////////////////////////////////////////
// Ucac4Star
//////////////////////////////////////////////////////////////////////
std::string	Ucac4Star::toString() const {
	return stringprintf("%s %8.4f %8.4f %6.3f", number.toString().c_str(),
		ra().hours(), dec().degrees(), mag());
}

//////////////////////////////////////////////////////////////////////
// Ucac4 Catalog
//////////////////////////////////////////////////////////////////////
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

#pragma pack(push, 1)
#define UCAC4_STAR struct ucac4_star

UCAC4_STAR
   {
   int32_t ra, spd;         /* RA/dec at J2000.0,  ICRS,  in milliarcsec */
   uint16_t mag1, mag2;     /* UCAC fit model & aperture mags, .001 mag */
   uint8_t mag_sigma;
   uint8_t obj_type, double_star_flag;
   int8_t ra_sigma, dec_sigma;    /* sigmas in RA and dec at central epoch */
   uint8_t n_ucac_total;      /* Number of UCAC observations of this star */
   uint8_t n_ucac_used;      /* # UCAC observations _used_ for this star */
   uint8_t n_cats_used;      /* # catalogs (epochs) used for prop motion */
   uint16_t epoch_ra;        /* Central epoch for mean RA, minus 1900, .01y */
   uint16_t epoch_dec;       /* Central epoch for mean DE, minus 1900, .01y */
   int16_t pm_ra;            /* prop motion, .1 mas/yr = .01 arcsec/cy */
   int16_t pm_dec;           /* prop motion, .1 mas/yr = .01 arcsec/cy */
   int8_t pm_ra_sigma;       /* sigma in same units */
   int8_t pm_dec_sigma;
   uint32_t twomass_id;        /* 2MASS pts_key star identifier */
   uint16_t mag_j, mag_h, mag_k;  /* 2MASS J, H, K_s mags,  in millimags */
   uint8_t icq_flag[3];
   uint8_t e2mpho[3];          /* 2MASS error photometry (in centimags) */
   uint16_t apass_mag[5];      /* in millimags */
   uint8_t apass_mag_sigma[5]; /* also in millimags */
   uint8_t yale_gc_flags;      /* Yale SPM g-flag * 10 + c-flag */
   uint32_t catalog_flags;
   uint8_t leda_flag;          /* LEDA galaxy match flag */
   uint8_t twomass_ext_flag;   /* 2MASS extended source flag */
   uint32_t id_number;
   uint16_t ucac2_zone;
   uint32_t ucac2_number;
   };
#pragma pack(pop)

#define	MARCSEC_to_RADIANS	(M_PI / (180 * 60 * 60 * 1000))

static Ucac4Star	UCAC4_to_Ucac4Star(uint16_t zone, uint32_t number,
				const UCAC4_STAR *star) {
	Ucac4Star	result(zone, number);
	result.catalog = 'U';
	result.catalognumber = zone * 100000000 + number;
	result.id_number = star->id_number;
	result.ra() = MARCSEC_to_RADIANS * star->ra;
	result.dec() = MARCSEC_to_RADIANS * star->spd - M_PI / 2;
	// Proper motion
	result.pm().ra() = MARCSEC_to_RADIANS * star->pm_ra / cos(result.dec());
	result.pm().dec() = MARCSEC_to_RADIANS * star->pm_dec;

	// magnitude
	result.mag() = star->mag1 * 0.001;
	result.mag2 = star->mag2 * 0.001;

	result.magsigma = star->mag_sigma * 0.001;
	result.obj_type = star->obj_type;
	result.double_star_flag = star->double_star_flag;
	result.pm_ra = star->pm_ra;
	result.pm_dec = star->pm_dec;
	result.pm_ra_sigma = star->pm_ra_sigma;
	result.pm_dec_sigma = star->pm_dec_sigma;
	result.mag_j = star->mag_j * 0.001;
	result.mag_h = star->mag_h * 0.001;
	result.mag_k = star->mag_k * 0.001;
	int	icf1 = star->catalog_flags % 100000000;
	result.hiptyc2 = (icf1 == 1) || (icf1 == 3);
	return result;
}

/* Note: sizeof( UCAC4_STAR) = 78 bytes */

Ucac4ZonePtr	Ucac4::zone(uint16_t zone) const {
	std::string	zfn = zonefilename(zone);
	return Ucac4ZonePtr(new Ucac4Zone(zone, zfn));
}

/**
 * \brief Get a particular zone in the cache
 */
Ucac4ZonePtr	Ucac4::getzone(uint16_t z) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving zone %hu", z);
	if ((cachedzone == NULL) || (cachedzone->zone() != z)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "opening zone");
		cachedzone = zone(z);
	} 
	return cachedzone;
}

/**
 * \brief find a particular star
 */
Ucac4Star	Ucac4::find(const RaDec& position) {
	// XXX imlementation missing
}

/**
 * \brief find a star based on the UCAC4 number
 */
Ucac4Star	Ucac4::find(const std::string& ucacnumber) {
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting star from zone %hu, number %u",
		zone, number);

	// open the zone file
	return getzone(zone)->get(number);
}

/**
 * \brief Retrieve all stars in a window
 */
Ucac4::starsetptr	Ucac4::find(const SkyWindow& window,
				const MagnitudeRange& magrange) {
	// prepare the empty result set
	starset	*result = new starset();
	starsetptr	resultptr(result);

	// find minimum an maximum zone numbers
	std::pair<double, double>	interval = window.decinterval();
	uint16_t	minzone = 1 + floor((interval.first + M_PI / 2) / (0.2 * M_PI / 180));
	uint16_t	maxzone = 1 + floor((interval.second + M_PI / 2) / (0.2 * M_PI / 180));
	if (maxzone > 900) {
		maxzone = 900;
	}

	// iterate through zone numbers
	uint32_t	counter = 0;
	for (uint16_t zoneno = minzone; zoneno <= maxzone; zoneno++) {
		Ucac4ZonePtr	z = zone(zoneno);
		z->add(resultptr, window, magrange);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%u stars found, %u in set", counter,
		result->size());

	// thats it
	return resultptr;
}

//////////////////////////////////////////////////////////////////////
// Ucac4Zone implementation
//////////////////////////////////////////////////////////////////////
Ucac4Zone::Ucac4Zone(uint16_t zone, const std::string& zonefilename)
		: MappedFile(zonefilename, sizeof(UCAC4_STAR)), _zone(zone) {
}

/**
 * \brief get a particular star from the zone
 */
Ucac4Star	Ucac4Zone::get(uint32_t number) const {
	if (number == 0) {
		throw std::runtime_error("cannot get star number 0");
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "getting star %u", number);
	std::string	line = MappedFile::get(number - 1);
	return UCAC4_to_Ucac4Star(_zone, number, (UCAC4_STAR *)line.data());
}

/**
 * \brief Get the first star number exceeding the ra
 */
uint32_t	Ucac4Zone::first(const Angle& ra) const {
	// get the last star an make sure the 
	Ucac4Star	laststar = get(nstars() - 1);
	if (laststar.ra() < ra) {
		return nstars();
	}

	// search in the interval
	uint32_t	l1 = 1, l2 = nstars();
	Angle	ra1 = get(l1).ra();
	Angle	ra2 = get(l2).ra();
	while ((l2 - l1) > 1) {
		uint32_t	l = (l1 + l2) / 2;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new subdivision: %lu", l);
		Angle	ra0 = get(l).ra();
		if (ra0 < ra) {
			l1 = l;
			ra1 = ra0;
		}
		if (ra <= ra0) {
			l2 = l;
			ra2 = ra0;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%u,%u = %f < %f <= %f",
			l1, l2, ra1.hours(), ra.hours(), ra2.hours());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "first: %lu", l2);
	return l2;
}

/**
 * \brief 
 */
Ucac4Zone::starsetptr	Ucac4Zone::find(const SkyWindow& window,
				const MagnitudeRange& magrange) {
	starset	*result = new starset();
	starsetptr	resultptr(result);
	return add(resultptr, window, magrange);
}

Ucac4Zone::starsetptr	Ucac4Zone::add(Ucac4::starsetptr set,
		const SkyWindow& window, const MagnitudeRange& magrange) {

	// get the maximum and minimum RA
	uint32_t	minindex = first(window.leftra());
	uint32_t	maxindex = first(window.rightra());
	if (minindex < maxindex) {
		for (uint32_t number = minindex; number < maxindex; number++) {
			Ucac4Star	star = get(number);
			if (magrange.contains(star.mag())) {
				set->insert(star);
			}
		}
	}
	if (maxindex < minindex) {
		for (uint32_t number = 1; number < maxindex; number++) {
			Ucac4Star	star = get(number);
			if (magrange.contains(star.mag())) {
				set->insert(star);
			}
		}
		for (uint32_t number = minindex; number < nstars(); number++) {
			Ucac4Star	star = get(number);
			if (magrange.contains(star.mag())) {
				set->insert(star);
			}
		}
	}

	// return stars
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%u stars from zone %hu", set->size(),
		_zone);
	return set;
}

} // namespace catalog
} // namespace astro
