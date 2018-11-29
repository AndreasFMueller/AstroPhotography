/*
 * SimFilterWheel.h -- FilterWheel simulator 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SimFilterWheel_h
#define _SimFilterWheel_h

#include <AstroCamera.h>
#include <SimLocator.h>
#include <AstroUtils.h>

namespace astro {
namespace camera {
namespace simulator {

class SimFilterWheel : public FilterWheel {
	SimLocator&	_locator;
	int	_currentposition;
	State	_currentstate;
	double	_changetime;
	void	checkstate();
public:
	SimFilterWheel(SimLocator& locator);
protected:
	virtual unsigned int	nFilters0() { return 5; }
public:
	virtual unsigned int	currentPosition();
	virtual void	select(size_t filterindex);
	virtual std::string	filterName(size_t filterindex);
	virtual State	getState();
};

} // namespace simulator
} // namespace camera
} // namespace astro

#endif /* _SimFilterWheel_h */
