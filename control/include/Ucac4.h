/*
 * Ucac4.h -- class declarations for Ucac4 star catalog
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Ucac4_h
#define _Ucac4_h

#include <AstroCoordinates.h>
#include <AstroCatalog.h>
#include <set>
#include <limits.h>
#include <string>
#include <MappedFile.h>

namespace astro {
namespace catalog {

/**
 * \brief Star number class for Ucac4 catalog
 */
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
	bool	operator==(const Ucac4StarNumber& other) const;
	bool	operator!=(const Ucac4StarNumber& other) const;
	bool	operator<(const Ucac4StarNumber& other) const;
};
std::ostream&	operator<<(std::ostream& out, const Ucac4StarNumber& star);

/**
 * \brief Star from the UC4 catalog
 */
class Ucac4Star : public Star {
public:
	// identification of a star
	uint32_t	id_number;
	Ucac4StarNumber	number;

	// position and errors
	float	ra_sigma, dec_sigma;

	// information about the star
	float	mag2;
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

	bool	operator<(const Ucac4Star& other) const {
		return number < other.number;
	}
};

/**
 * \brief A zone of the UC4 catalog
 */
class Ucac4Zone : public MappedFile {
	uint16_t	_zone;
public:
	uint16_t	zone() const { return _zone; }
	uint32_t	nstars() const { return nrecords(); }
public:
	typedef std::set<Ucac4Star>	starset;
	typedef std::shared_ptr<starset>	starsetptr;
	Ucac4Zone(uint16_t zone, const std::string& zonefilename);
	Ucac4Star	get(uint32_t number) const;
	uint32_t	first(const Angle& ra) const;
	starsetptr	find(const SkyWindow& window,
				const MagnitudeRange& magrange);
	starsetptr	add(starsetptr set, const SkyWindow& window,
				const MagnitudeRange& magrange);
};
typedef std::shared_ptr<Ucac4Zone>	Ucac4ZonePtr;

/**
 * \brief UC4 Catalog
 */
class Ucac4 {
	std::string	_directory;
	Ucac4Star	find(uint16_t zone, uint32_t number);
	std::string	zonefilename(uint16_t zone) const;
	std::string	indexfilename() const;
	Ucac4ZonePtr	cachedzone;
	Ucac4ZonePtr	getzone(uint16_t zone);
public:
	typedef std::set<Ucac4Star>	starset;
	typedef std::shared_ptr<starset>	starsetptr;
	Ucac4(const std::string& directory);
	Ucac4ZonePtr	zone(uint16_t zone) const;
	Ucac4Star	find(const RaDec& position);
	Ucac4Star	find(const std::string& ucacnumber);
	Ucac4Star	find(const Ucac4StarNumber& number);
	starsetptr	find(const SkyWindow& window,
				const MagnitudeRange& magrange);
};
typedef std::shared_ptr<Ucac4>	Ucac4Ptr;

} // namespace catalog
} // namespace astro

#endif /* _Ucac4_h */
