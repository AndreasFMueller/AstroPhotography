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
	std::recursive_mutex	_sim_mutex; // protect the state variables
	std::condition_variable	_sim_condition;
	RaDec			_target;	// where the movement is headed
	GreatCircle		_greatcircle;
	double			_when;
	static const double	_movetime;
	std::thread		_sim_thread;
	static void	main(SimMount *mount) noexcept;
	virtual void	move();
public:
	SimMount(/*SimLocator &locator*/);
	virtual ~SimMount();
	virtual RaDec	getRaDec();
	virtual AzmAlt	getAzmAlt();
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
