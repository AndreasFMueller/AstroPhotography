/*
 * Ucac4Zone.cpp -- Ucac4 zone implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Ucac4.h"
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <includes.h>

namespace astro {
namespace catalog {

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

static Ucac4Star	UCAC4_to_Ucac4Star(uint16_t zone, uint32_t index,
				const UCAC4_STAR *star) {
	Ucac4Star	result(zone, index);
	result.catalog('U');
	result.id_number = star->id_number;
	result.ra() = MARCSEC_to_RADIANS * star->ra;
	result.dec() = MARCSEC_to_RADIANS * star->spd - M_PI / 2;
	// Proper motion
	result.pm().ra() = MARCSEC_to_RADIANS * star->pm_ra / cos(result.dec());
	result.pm().dec() = MARCSEC_to_RADIANS * star->pm_dec;

	// magnitude
	result.mag(star->mag1 * 0.001);
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
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "new subdivision: %lu", l);
		Angle	ra0 = get(l).ra();
		if (ra0 < ra) {
			l1 = l;
			ra1 = ra0;
		}
		if (ra <= ra0) {
			l2 = l;
			ra2 = ra0;
		}
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "%u,%u = %f < %f <= %f",
		//	l1, l2, ra1.hours(), ra.hours(), ra2.hours());
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

Ucac4Zone::starsetptr	Ucac4Zone::add(Ucac4Zone::starsetptr set,
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

unsigned long	Ucac4Zone::numberOfStars() {
	return nrecords();
}

bool	Ucac4Zone::touches(const SkyWindow& window) const {
	return Ucac4::touches(_zone, window);
}

} // namespace catalog
} // namespace astro
