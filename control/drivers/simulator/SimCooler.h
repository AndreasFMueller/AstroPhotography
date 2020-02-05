/*
 * SimCooler.h -- Cooler simulator
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SimCooler_h
#define _SimCooler_h

#include <SimLocator.h>

namespace astro {
namespace camera {
namespace simulator {

class SimCooler : public Cooler {
	SimLocator&	_locator;
	double	laststatechange;
	Temperature	lasttemperature;
	bool	on;
public:
	SimCooler(SimLocator& locator);
	virtual Temperature	getActualTemperature();
protected:
	virtual void	setTemperature(float _temperature);
public:
	virtual void	setOn(bool onoff);
	virtual bool	isOn() { return on; }
	int	belowambient();
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
