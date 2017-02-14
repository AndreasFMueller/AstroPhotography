/*
 * AtikCooler.h -- Atik Cooler class
 *
 * (c) 2016 Prof Dr Andreas Müller
 */
#ifndef _AtikCooler_h
#define _AtikCooler_h

#include <atikccdusb.h>
#include <AstroCamera.h>

namespace astro {
namespace camera {
namespace atik {

class AtikCooler : public Cooler {
	::AtikCamera	*_camera;
	int	_tempSensorCount;
public:
	AtikCooler(::AtikCamera*);
	virtual ~AtikCooler();
	virtual float	getSetTemperature();
	virtual float	getActualTemperature();
	virtual void	setTemperature(const float temperature);
	virtual bool	isOn();
	virtual void	setOn(bool onoff);
};

} // namespace atik
} // namespace camera 
} // namespace astro

#endif /* _AtikCooler_h */
