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
	bool	_terminate;
	std::thread		_thread; // XXX movement monitoring thread
	std::mutex		_mutex;
	std::condition_variable	_cond;
public:
	SimFocuser(SimLocator& locator);
	virtual ~SimFocuser();
	long	min();
	long	max();
	long	current();
	long	backlash();
	virtual void	set(long value);
	double	radius();
	void	randomposition();	
private:
	static void	main(SimFocuser* focuser) noexcept;
	void	run();	// XXX main method for movement monitoring thread
};

} // namespace simulator
} // namespace camera
} // namespace astro

#endif /* _SimFocuser_h */
