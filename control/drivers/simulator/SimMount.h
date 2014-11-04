/*
 * SimMount.h -- simulated mount of the simulator camera
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SimMount_h
#define _SimMount_h

#include <AstroDevice.h>
#include <SimLocator.h>

namespace astro {
namespace camera {
namespace simulator {

class SimMount : public astro::device::Mount {
	SimLocator&	_locator;
public:
	astro::device::Mount::state_type	state();
	SimMount(SimLocator &locator);
	RaDec	getRaDec();
	AzmAlt	getAzmAlt();
	void	Goto(const RaDec& radec);
	void	Goto(const AzmAlt& azmalt);
	void	cancel();
};

} // namespace simulator
} // namespace camera 
} // namespace astro

#endif /* _SimMount_h */
