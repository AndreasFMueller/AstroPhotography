/*
 * Tycho2.h -- Tycho2 star catalog declarations
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Tycho2_h
#define _Tycho2_h

#include <AstroCatalog.h>
#include <string>
#include <MappedFile.h>

namespace astro {
namespace catalog {

/**
 * \brief Tycho2 Star
 */
class Tycho2Star : public Star {
	void	setup(const std::string& line);
	int	_hip;
public:
	bool	isHipparcosStar() const { return _hip >= 0; }
	int	hip() const { return _hip; }
	Tycho2Star(const std::string& line);
};

/**
 * \brief Tycho2 catalog 
 */
class Tycho2 : public MappedFile {
	std::string	_filename;
public:
	unsigned int	nstars() const { return nrecords(); }
public:
	typedef std::set<Tycho2Star>	starset;
	typedef std::shared_ptr<starset>	starsetptr;
	Tycho2(const std::string& filename);
	Tycho2Star	find(unsigned int index) const;
	starset	find(const SkyWindow& window,
			const MagnitudeRange& magrange) const;
};

} // namespace catalog
} // namespace astro

#endif /* _Tycho2_h */
