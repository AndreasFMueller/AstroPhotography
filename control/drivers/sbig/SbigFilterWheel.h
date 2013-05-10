/*
 * SbigFilterWheel.h -- SBIG filter wheel implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SbigCamera.h>

namespace astro {
namespace camera {
namespace sbig {

class SbigFilterWheelTimeout : public std::runtime_error {
public:
	SbigFilterWheelTimeout(const char *cause) : std::runtime_error(cause) {}
};

class SbigFilterWheel : public FilterWheel {
	SbigCamera&	camera;
	unsigned int	npositions;
	unsigned int	currentindex;
	void	init();
	void	wait();
public:
	SbigFilterWheel(SbigCamera& camera);
	virtual ~SbigFilterWheel();
	virtual unsigned int	nFilters();
	virtual unsigned int	currentPosition();
	virtual void	select(size_t filterindex);
	virtual std::string	filterName(size_t filterindex);
};

} // namespace sbig
} // namespace camera
} // namespace astro
