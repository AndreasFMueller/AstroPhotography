/*
 * NiceCooler.h -- wrapper fro coolers
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NiceCooler_h
#define _NiceCooler_h

#include <AstroCamera.h>
#include <camera.h>
#include <NiceDevice.h>

namespace astro {
namespace camera {
namespace nice {

class NiceCooler : public Cooler, public NiceDevice {
	snowstar::CoolerPrx	_cooler;
public:
	NiceCooler(snowstar::CoolerPrx cooler, const DeviceName& devicename);
	virtual ~NiceCooler();

	virtual float getSetTemperature();
	virtual float getActualTemperature();
	void	setTemperature(float temperature);
	
	virtual bool	isOn();
	virtual void	setOn(bool onoff);

	virtual bool	hasDewHeater();
	virtual float	dewHeater();
	virtual void	dewHeater(float dewheatervalue);
	virtual std::pair<float, float>	dewHeaterRange();
};

} // namespace nice
} // namespace camera
} // namespace astro

#endif /* _NiceCooler_h */
