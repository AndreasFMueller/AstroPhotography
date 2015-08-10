/*
 * AdaptiveOpticsI.h -- ICE AdaptiveOptics wrapper class definition
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AdaptiveOptics_h
#define _AdaptiveOptics_h

#include <device.h>
#include <AstroCamera.h>
#include <DeviceI.h>

namespace snowstar {

class AdaptiveOpticsI : virtual public AdaptiveOptics, virtual public DeviceI {
	astro::camera::AdaptiveOpticsPtr	_ao;
public:
	AdaptiveOpticsI(astro::camera::AdaptiveOpticsPtr ao);
	virtual ~AdaptiveOpticsI();
	virtual void	set(const Point& position, const Ice::Current& current);
	virtual Point	get(const Ice::Current& current);
	virtual bool	hasGuiderPort(const Ice::Current& current);
	virtual GuiderPortPrx	getGuiderPort(const Ice::Current& current);	
	virtual void	center(const Ice::Current& current);
};

} // namespace snowstar

#endif /* _AdaptiveOptics_h */
