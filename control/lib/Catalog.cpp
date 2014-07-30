/*
 * Catalog.cpp -- Catalog access implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <includes.h>

namespace astro {
namespace catalog {

//////////////////////////////////////////////////////////////////////
// SkyWindow
//////////////////////////////////////////////////////////////////////

static double	reduce(double x, double left) {
	return x - 2 * M_PI * floor((x - left) / (2 * M_PI));
}

/**
 * \brief find out whether a position is within the window
 */
bool	SkyWindow::contains(const RaDec& position) const {
	// check right ascension
	double	left = _center.ra().radians() - _rawidth / 2.;
	double	right = _center.ra().radians() + _rawidth / 2.;
	double	ra = reduce(position.ra().radians(), left);
	if (ra > right) {
		return false;
	}

	// check declination
	double	top = _center.dec().radians() - _decheight / 2.;
	double	bottom = _center.dec().radians() + _decheight / 2.;
	double	dec = reduce(position.dec().radians(), bottom);
	if (dec > top) {
		return false;
	}

	// position is inside the window
	return true;
}

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

bool	Ucac4StarNumber::operator==(const Ucac4StarNumber& other) {
	return (_zone == other._zone) && (_number == other._number);
}

bool	Ucac4StarNumber::operator!=(const Ucac4StarNumber& other) {
	return !(*this == other);
}

std::ostream&	operator<<(std::ostream& out, const Ucac4StarNumber& star) {
	out << star.toString();
	return out;
}

//////////////////////////////////////////////////////////////////////
// Ucac4Star
//////////////////////////////////////////////////////////////////////
std::string	Ucac4Star::toString() const {
	return stringprintf("%s %8.4f %8.4f", number.toString().c_str(),
		position.ra().hours(), position.dec().degrees());
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking file %s", filename.c_str());
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
	result.id_number = star->id_number;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "id: %d\n", result.id_number);
	result.position.ra() = MARCSEC_to_RADIANS * star->ra;
	result.position.dec() = MARCSEC_to_RADIANS * star->spd - M_PI / 2;
	result.mag1 = star->mag1 * 0.001;
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
std::set<Ucac4Star>	Ucac4::find(const SkyWindow& window,
				float minimum_magnitude) {
	std::set<Ucac4Star>	result;
	return result;
}

//////////////////////////////////////////////////////////////////////
// Ucac4Zone implementation
//////////////////////////////////////////////////////////////////////
Ucac4Zone::Ucac4Zone(uint16_t zone, const std::string& zonefilename)
		: _zone(zone) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create zone %hu from file %s",
		zone, zonefilename.c_str());
	// find the file
	struct stat	sb;
	if (stat(zonefilename.c_str(), &sb) < 0) {
		std::string	msg = stringprintf("cannot stat zone file "
			"'%s': %s", zonefilename.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	data_len = sb.st_size;
	nstars = sb.st_size / 78;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "zone '%s' has %d stars",
		zonefilename.c_str(), nstars);

	// open the file for reading
	int	fd = open(zonefilename.c_str(), O_RDONLY);
	if (fd < 0) {
		std::string	msg = stringprintf("cannot open zone file "
			"'%s': %s", zonefilename.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// map the file
	data_ptr = mmap(NULL, sb.st_size, PROT_READ, MAP_FILE | MAP_PRIVATE,
		fd, 0);
	if ((void *)(-1) == data_ptr) {
		std::string	msg = stringprintf("cannot map file '%s': %s",
			zonefilename.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	close(fd);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "zone data mapped to %p", data_ptr);
}

/**
 * \brief Destroy the zone, unmap the file
 */
Ucac4Zone::~Ucac4Zone() {
	if (data_ptr) {
		munmap(data_ptr, data_len);
	}
}

/**
 * \brief get a particular star from the zone
 */
Ucac4Star	Ucac4Zone::get(uint32_t number) const {
	if (number > nstars) {
		std::string	msg = stringprintf("%ul exceeds number of "
			"stars %d", number, nstars);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return UCAC4_to_Ucac4Star(_zone, number,
			&((UCAC4_STAR *)data_ptr)[number]);
}

/**
 * \brief Get the first star number exceeding the ra
 */
uint32_t	Ucac4Zone::first(const Angle& ra) const {
	uint32_t	l1 = 0, l2 = nstars;
}

/**
 * \brief 
 */
std::set<Ucac4Star>	Ucac4Zone::find(const SkyWindow& window,
				float minimum_magnitude) {
	std::set<Ucac4Star>	result;
	return result;
}

} // namespace catalog
} // namespace astro
