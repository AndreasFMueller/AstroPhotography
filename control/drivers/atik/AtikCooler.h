/*
 * AtikCooler.h -- Atik Cooler class
 *
 * (c) 2016 Prof Dr Andreas Müller
 */
#ifndef _AtikCooler_h
#define _AtikCooler_h

#include <atikccdusb.h>
#include <AstroCamera.h

namespace astro {
namespace camera {
namespace atik {

class AtikCooler : public Cooler {
public:
	AtikCooler(::AtikCamera*);
	virtual float	getSetTemperature();
	virtual float	getActualTemperature();
	virtual void	setTemeprature(const float temperature);
	vitual bool	isOn();
	virtual void	setOn(bool onoff);
};

} // namespace atik
} // namespace camera 
} // namespace astro

#endif /* _AtikCooler_h */
