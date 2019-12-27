/*
 * CoolerI.h -- ICE Cooler wrapper
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CoolerI_h
#define _CoolerI_h

#include <camera.h>
#include <AstroCamera.h>
#include <DeviceI.h>

namespace snowstar {

class CoolerI : virtual public Cooler, virtual public DeviceI {
	astro::camera::CoolerPtr	_cooler;
public:
	CoolerI(astro::camera::CoolerPtr cooler);
	virtual ~CoolerI();
	float	getSetTemperature(const Ice::Current& current);
	float	getActualTemperature(const Ice::Current& current);
	void	setTemperature(float temperature, const Ice::Current& current);
	bool	isOn(const Ice::Current& current);
	void	setOn(bool onoff, const Ice::Current& current);
static	CoolerPrx	createProxy(const std::string& coolername,
				const Ice::Current& current);
	// dew heater stuff
	bool	hasDewHeater(const Ice::Current& current);
	float	getDewHeater(const Ice::Current& current);
	void	setDewHeater(float dewheatervalue, const Ice::Current& current);
	Interval	dewHeaterRange(const Ice::Current& current);
};

} // namespace snowstar

#endif /* _CoolerI_h */
