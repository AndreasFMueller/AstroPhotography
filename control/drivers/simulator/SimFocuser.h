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
	unsigned short	target;
	unsigned short	_value;
	double  reference();
	unsigned short  variance();
public:
	SimFocuser(SimLocator& locator);
	virtual ~SimFocuser() { }
	virtual unsigned short  min();
	virtual unsigned short  max();
	virtual unsigned short  current();
	virtual void	set(unsigned short value);
	double	radius();
	void	randomposition();	
};

} // namespace simulator
} // namespace camera
} // namespace astro

#endif /* _SimFocuser_h */
