/*
 * SimFocuser.h -- simulator focuser definition
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SimFocuser_h
#define _SimFocuser_h

#include <AstroCamera.h>
#include <SimLocator.h>

namespace astro {
namespace camera {
namespace simulator {

class SimFocuser : public Focuser {
	SimLocator&	_locator;
	double	lastset;
	long	target;
	long	_value;
	double  reference();
	long	variance();
public:
	SimFocuser(SimLocator& locator);
	virtual ~SimFocuser() { }
	long	min();
	long	max();
	long	current();
	long	backlash();
	virtual void	set(long value);
	double	radius();
	void	randomposition();	
};

} // namespace simulator
} // namespace camera
} // namespace astro

#endif /* _SimFocuser_h */
