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

/**
 * \brief The simulated filterwheel
 */
class SimFilterWheel : public FilterWheel {
	SimLocator&	_locator;
	int	_currentposition;
	int	_nextposition;
	State	_currentstate;
	std::thread	_thread;
	std::mutex	_mutex;
	std::condition_variable	_cond;
	bool	_terminate;
public:
	SimFilterWheel(SimLocator& locator);
	virtual ~SimFilterWheel();
private:
	static void	main(SimFilterWheel *filterwheel) noexcept;
	void	run();
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
