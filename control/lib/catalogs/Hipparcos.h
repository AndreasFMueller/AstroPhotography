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
#include "CatalogIterator.h"

namespace astro {
namespace catalog {

/**
 * \brief Hipparcos star catalog star
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
 * \brief Hipparcos Star catalog
 */
class Hipparcos : public MappedFile, public Catalog {
	Precession	precession;
public:
	typedef std::map<unsigned int, HipparcosStar>	starmap_t;
private:
	starmap_t	stars;
public:
	Hipparcos(const std::string& filename);
	virtual ~Hipparcos();
	HipparcosStar	find(unsigned int hip) const;
	virtual Star	find(const std::string& name);
	virtual starsetptr	find(const SkyWindow& window,
				const MagnitudeRange& magrange);
	virtual unsigned long	numberOfStars();
	virtual CatalogIterator	begin();
};

/**
 * \brief Iterator for the Hipparcos catalog
 */
class HipparcosIterator : public IteratorImplementation {
	Hipparcos::starmap_t&	_stars;
	Hipparcos::starmap_t::iterator	_i;
public:
	HipparcosIterator(Hipparcos::starmap_t& stars);
	virtual Star	operator*();
	bool	operator==(const HipparcosIterator& other) const;
	virtual bool	operator==(const IteratorImplementation& other) const;
	virtual std::string	toString() const;
	virtual void	increment();
};

} // namespace catalog 
} // namespace astro

#endif /* _Hipparcos_h */
