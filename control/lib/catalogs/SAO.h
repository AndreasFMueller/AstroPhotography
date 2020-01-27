/*
 * SAO.h -- SAO catalog
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SAO_h
#define _SAO_h

#include <AstroCatalog.h>
#include <string>
#include <vector>
#include <map>
#include "MappedFile.h"
#include "CatalogIterator.h"

namespace astro {
namespace catalog {

/**
 * \brief SAO star catalog star
 */
class SAOStar : public Star {
public:
	SAOStar(const std::string& line);
	unsigned int	sao;
	bool	operator<(const SAOStar& other) const;
	bool	operator>(const SAOStar& other) const;
	bool	operator<=(const SAOStar& other) const;
	bool	operator>=(const SAOStar& other) const;
	virtual std::string	toString() const;
};

/**
 * \brief SAO Star catalog
 */
class SAO : public MappedFile, public Catalog {
	Precession	precession;
public:
	typedef std::map<unsigned int, SAOStar>	starmap_t;
private:
	starmap_t	stars;
public:
	SAO(const std::string& filename);
	virtual ~SAO();
	SAOStar	find(unsigned int sao) const;
	virtual Star	find(const std::string& name);
	virtual starsetptr	find(const SkyWindow& window,
				const MagnitudeRange& magrange);
	virtual unsigned long	numberOfStars();
	virtual CatalogIterator	begin();
};

/**
 * \brief Iterator for the SAO catalog
 */
class SAOIterator : public IteratorImplementation {
	SAO::starmap_t&	_stars;
	SAO::starmap_t::iterator	_i;
public:
	SAOIterator(SAO::starmap_t& stars);
	virtual Star	operator*();
	bool	operator==(const SAOIterator& other) const;
	virtual bool	operator==(const IteratorImplementation& other) const;
	virtual std::string	toString() const;
	virtual void	increment();
};

} // namespace catalog 
} // namespace astro

#endif /* _SAO_h */
