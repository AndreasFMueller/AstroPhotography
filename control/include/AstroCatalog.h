/*
 * AstroCatalog.h --
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroCatalog_h
#define _AstroCatalog_h

#include <AstroCoordinates.h>
#include <set>
#include <limits.h>
#include <string>

namespace astro {
namespace catalog {

/**
 * \brief Window on the sky, used to select stars from the catalog
 */
class SkyWindow {
	RaDec	_center;
public:
	const RaDec&	center() const { return _center; }
private:
	double	_rawidth;
	double	_decheight;
public:
	double	rawdith() const { return _rawidth; }
	double	decheight() const { return _decheight; }
public:
	SkyWindow(const RaDec& center, double rawidth, double decheight);
	bool	contains(const RaDec& position) const;
};

/**
 * \brief Star base class
 */
class Star {
public:
	RaDec	position;
};

class Ucac4StarNumber {
private:
	uint16_t	_zone;
	uint32_t	_number;
public:
	const uint16_t&	zone() const { return _zone; }
	const uint32_t&	number() const { return _number; }
	
	Ucac4StarNumber(uint16_t zone, uint16_t number)
		: _zone(zone), _number(number) { }
	Ucac4StarNumber(const std::string& starnumber);
	std::string	toString() const;
	bool	operator==(const Ucac4StarNumber& other);
	bool	operator!=(const Ucac4StarNumber& other);
};

/**
 * \brief Star from the UC4 catalog
 */
class Ucac4Star : public Star {
public:
	// identification of a star
	uint32_t	id_number;
	Ucac4StarNumber	number;

	// position and errors
	RaDec	position;
	float	ra_sigma, dec_sigma;

	// information about the star
	float	mag1, mag2;
	float	magsigma;
	uint8_t	obj_type;
	uint8_t	double_star_flag;
	// proper motion
	float	pm_ra, pm_dec, pm_ra_sigma, pm_dec_sigma;

	// 2MASS information
	uint32_t	twmass_id;
	float	mag_j, mag_h, mag_k;

	// constructor
	Ucac4Star(const std::string& starnumber) : number(starnumber) { }
	Ucac4Star(uint16_t zone, uint32_t _number) : number(zone, _number) { }

	// additional methods
	std::string	toString() const;
};

/**
 * \brief UC4 Catalog
 */
class Ucac4 {
	std::string	_directory;
	Ucac4Star	find(uint16_t zone, uint32_t number);
	std::string	zonefilename(uint16_t zone) const;
	std::string	indexfilename() const;
public:
	Ucac4(const std::string& directory);
	Ucac4Star	find(const RaDec& position);
	Ucac4Star	find(const std::string& ucacnumber);
	Ucac4Star	find(const Ucac4StarNumber& number);
	std::set<Ucac4Star>	find(const SkyWindow& window,
		float minimum_magnitude = -std::numeric_limits<float>::min());
};
typedef std::shared_ptr<Ucac4>	Ucac4Ptr;

} // namespace catalog
} // namespace astro

#endif /* _AstroCatalog_h */
