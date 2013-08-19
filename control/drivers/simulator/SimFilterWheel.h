/*
 * SimFilterWheel.h -- FilterWheel simulator 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SimFilterWheel_h
#define _SimFilterWheel_h

#include <AstroCamera.h>
#include <SimLocator.h>

namespace astro {
namespace camera {
namespace simulator {

class SimFilterWheel : public FilterWheel {
	SimLocator&	_locator;
	int	_currentPosition;
public:
	SimFilterWheel(SimLocator& locator);
	virtual unsigned int	nFilters() { return 5; }
	virtual unsigned int	currentPosition() { return _currentPosition; }
	virtual void	select(size_t filterindex);
	virtual std::string	filterName(size_t filterindex);
};

} // namespace simulator
} // namespace camera
} // namespace astro

#endif /* _SimFilterWheel_h */
