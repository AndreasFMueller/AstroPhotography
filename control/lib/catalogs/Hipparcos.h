/*
 * Hipparcos.h -- Hipparcos catalog
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Hipparcos_h
#define _Hipparcos_h

#include <AstroCatalog.h>
#include <string>
#include <vector>
#include <map>
#include "MappedFile.h"

namespace astro {
namespace catalog {

/**
 * \brief Bright star catalog star
 */
class HipparcosStar : public Star {
public:
	HipparcosStar(const std::string& line);
	unsigned int	hip;
	bool	operator<(const HipparcosStar& other) const;
	bool	operator>(const HipparcosStar& other) const;
	bool	operator<=(const HipparcosStar& other) const;
	bool	operator>=(const HipparcosStar& other) const;
	virtual std::string	toString() const;
};

/**
 * \brief Bright Star catalog
 */
class Hipparcos : public MappedFile,
		public std::map<unsigned int, HipparcosStar> {
	std::string	_filename;
public:
	typedef std::set<HipparcosStar>	starset;
	typedef std::shared_ptr<starset>	starsetptr;
	Hipparcos(const std::string& filename);
	HipparcosStar	find(unsigned int hip) const;
	starsetptr	find(const SkyWindow& window,
			const MagnitudeRange& magrange) const;
};

} // namespace catalog 
} // namespace astro

#endif /* _Hipparcos_h */
