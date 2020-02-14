/*
 * SimCooler.h -- Cooler simulator
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SimCooler_h
#define _SimCooler_h

#include <SimLocator.h>
#include <thread>

namespace astro {
namespace camera {
namespace simulator {

class SimCooler : public Cooler {
	SimLocator&	_locator;
	double	laststatechange;		// time of last reported change
	Temperature	lasttemperature;	// last reported temperature
	bool	on;
	void	updateTemperature();
	void	sendInfo();
	// thread stuff for temperature polling and callback generation
	std::thread		_thread;
	std::condition_variable_any	_cond;
	std::recursive_mutex	_mutex;
	bool			_terminate;
public:
	SimCooler(SimLocator& locator);
	virtual ~SimCooler();
	virtual Temperature	getActualTemperature();
protected:
	virtual void	setTemperature(float _temperature);
public:
	virtual void	setOn(bool onoff);
	virtual bool	isOn() { return on; }
	int	belowambient();
	void	run();
private:
	float	_dewheatervalue;
public:
	virtual bool	hasDewHeater();
	virtual float	dewHeater();
	virtual void	dewHeater(float d);
	virtual std::pair<float, float>	dewHeaterRange();
};

} // namespace simulator
} // namespace camera
} // namespace astro

#endif /* _SimCooler_h */
