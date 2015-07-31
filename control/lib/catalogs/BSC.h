/*
 * BSC.h -- bright star catalog
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _BSC_h
#define _BSC_h

#include <AstroCatalog.h>
#include "CatalogIterator.h"
#include <string>
#include <vector>
#include <map>

namespace astro {
namespace catalog {

/**
 * \brief Bright star catalog star
 */
class BSCStar : public Star {
public:
	BSCStar(const char *line);
	unsigned short	number;	// BSC object number
	unsigned int	sao;
	std::vector<std::string>	notes;
	bool	operator<(const BSCStar& other) const;
	bool	operator>(const BSCStar& other) const;
	bool	operator<=(const BSCStar& other) const;
	bool	operator>=(const BSCStar& other) const;
};

/**
 * \brief Bright Star catalog
 */
class BSC : public Catalog {
	std::string	_filename;
	std::string	_notesfile;
public:
	typedef std::map<unsigned short, BSCStar>	starmap_t;
private:
	starmap_t	stars;
	void	setup();
public:
	BSC(const std::string& filename, const std::string& notesfile);
	BSC(const std::string& basedir);
	virtual ~BSC();

	BSCStar	find(int number);
	virtual Star	find(const std::string& name);
	virtual starsetptr	find(const SkyWindow& window,
					const MagnitudeRange& magrange);
	virtual unsigned long	numberOfStars();

	virtual CatalogIterator	begin();
	virtual CatalogIterator	end();
};

/**
 * \brief Iterator for BSC catalog
 */
class BSCIterator : public IteratorImplementation {
	BSC::starmap_t&	_stars;
	BSC::starmap_t::iterator	_i;
public:
	BSCIterator(BSC::starmap_t& stars, bool begin_or_end);
	virtual Star	operator*();
	bool	operator==(const BSCIterator& other) const;
	virtual bool	operator==(const IteratorImplementation& other) const;
	virtual std::string	toString() const;
	virtual void	increment();
};

} // namespace catalog 
} // namespace astro

#endif /* _BSC_h */
