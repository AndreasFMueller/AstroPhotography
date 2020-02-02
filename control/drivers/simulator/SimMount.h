/*
 * SimMount.h -- simulated mount of the simulator camera
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SimMount_h
#define _SimMount_h

#include <AstroDevice.h>
#include <SimLocator.h>
#include <thread>
#include <CancellableWork.h>

namespace astro {
namespace camera {
namespace simulator {

class SimMount : public astro::device::Mount {
	//SimLocator&	_locator;
	// the current direction the telescope points to
	RaDec	_direction;
	void	direction(const RaDec& d);
	RaDec	direction();
	// dynamic movement: target and time when the target will be reached
	std::recursive_mutex	_mutex; // protext the state variables
	RaDec	_target;	// where the movement is headed
	double	_when;
	std::thread	*_thread;
public:
	void	move();
	astro::device::Mount::state_type	state();
	SimMount(/*SimLocator &locator*/);
	virtual ~SimMount();
	RaDec	getRaDec();
	AzmAlt	getAzmAlt();
	void	Goto(const RaDec& radec);
	void	Goto(const AzmAlt& azmalt);
	void	cancel();
	virtual bool	hasGuideRates();
	virtual RaDec	getGuideRates();
};

} // namespace simulator
} // namespace camera 
} // namespace astro

#endif /* _SimMount_h */
