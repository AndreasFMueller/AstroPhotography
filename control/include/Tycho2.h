/*
 * Tycho2.h -- Tycho2 star catalog declarations
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <string>

namespace astro {
namespace catalog {

/**
 * \brief Tycho2 Star
 */
class Tycho2Star : public Star {
	void	setup(const std::string& line);
public:
	Tycho2Star(const char *line);
	Tycho2Star(const std::string& line);
};

/**
 * \brief Tycho2 catalog 
 */
class Tycho2 {
	std::string	_filename;
	unsigned int	_nstars;
public:
	unsigned int	nstars() const { return _nstars; }
private:
	size_t	data_len;
	char	*data_ptr;
public:
	Tycho2(const std::string& filename);
	~Tycho2();
	Tycho2Star	find(unsigned int index) const;
	std::set<Tycho2Star>	find(const SkyWindow& window,
					double minimum_magnitude);
};

} // namespace catalog
} // namespace astro
