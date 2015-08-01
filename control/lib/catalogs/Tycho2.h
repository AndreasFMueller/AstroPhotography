/*
 * Tycho2.h -- Tycho2 star catalog declarations
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Tycho2_h
#define _Tycho2_h

#include <AstroCatalog.h>
#include <string>
#include "MappedFile.h"
#include "CatalogIterator.h"

namespace astro {
namespace catalog {

/**
 * \brief Tycho2 Star
 */
class Tycho2Star : public Star {
	void	setup(unsigned int index, const std::string& line);
public:
	Tycho2Star(const std::string& line, unsigned int index = 0);
};

/**
 * \brief Tycho2 catalog 
 */
class Tycho2 : public MappedFile, public Catalog {
public:
	unsigned int	nstars() const { return nrecords(); }

	typedef std::map<std::string, int>	namemap_t;
private:
	namemap_t	names;
	std::string	key(int index) const;
	int	index(const std::string& name);
public:
	Tycho2(const std::string& filename);
	virtual ~Tycho2();

	// find star by index number
	Tycho2Star	find(unsigned int index) const;

	// standard find interface
	virtual Star	find(const std::string& name);
	virtual starsetptr	find(const SkyWindow& window,
				const MagnitudeRange& magrange);
	// catalog size
	virtual unsigned long	numberOfStars();

	// iterators
	virtual CatalogIterator	begin();
};

/**
 * \brief Tycho2 iterator
 */
class Tycho2Iterator : public IteratorImplementation {
	unsigned int	_index;
	Tycho2&	_catalog;
public:
	Tycho2Iterator(unsigned int index, Tycho2& catalog);
	virtual ~Tycho2Iterator();
	virtual Star	operator*();
	virtual bool	operator==(const IteratorImplementation& other) const;
	bool	operator==(const Tycho2Iterator& other) const;
	virtual std::string	toString() const;
	virtual void	increment();
};

} // namespace catalog
} // namespace astro

#endif /* _Tycho2_h */
