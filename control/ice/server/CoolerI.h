/*
 * CoolerI.h -- ICE Cooler wrapper
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CoolerI_h
#define _CoolerI_h

#include <camera.h>
#include <AstroCamera.h>

namespace snowstar {

class CoolerI : public Cooler {
	astro::camera::CoolerPtr	_cooler;
public:
	CoolerI(astro::camera::CoolerPtr& cooler) : _cooler(cooler) { }
	virtual ~CoolerI() { }
	float	getSetTemperature(const Ice::Current& current);
	float	getActualTemperature(const Ice::Current& current);
	void	setTemperature(float temperature, const Ice::Current& current);
	bool	isOn(const Ice::Current& current);
	void	setOn(bool onoff, const Ice::Current& current);
	std::string	getName(const Ice::Current& current);
};

} // namespace snowstar

#endif /* _CoolerI_h */
