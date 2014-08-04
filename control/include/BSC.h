/*
 * BSC.h -- bright star catalog
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _BSC_h
#define _BSC_h

#include <AstroCatalog.h>
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
	unsigned short	number;
	std::string	name;
	unsigned int	sao;
	std::vector<std::string>	notes;
	bool	operator<(const BSCStar& other) const;
	bool	operator>(const BSCStar& other) const;
	bool	operator<=(const BSCStar& other) const;
	bool	operator>=(const BSCStar& other) const;
	virtual std::string	toString() const;
};

/**
 * \brief Bright Star catalog
 */
class BSC {
	std::string	_filename;
	std::string	_notesfile;
	std::map<unsigned short, BSCStar>	stars;
public:
	typedef std::set<BSCStar>	starset;
	typedef std::shared_ptr<starset>	starsetptr;
	BSC(const std::string& filename, const std::string& notesfile);
	const BSCStar&	find(int number) const;
	starset	find(const SkyWindow& window,
			const MagnitudeRange& magrange) const;
};

} // namespace catalog 
} // namespace astro

#endif /* _BSC_h */
