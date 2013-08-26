/*
 * NetCooler.h -- corba/network based cooler driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NetCooler_h
#define _NetCooler_h

#include <AstroCamera.h>
#include <device.hh>

namespace astro {
namespace camera {
namespace net {

class NetCooler : public Cooler {
	Astro::Cooler_var	_cooler;
public:
	NetCooler(Astro::Cooler_var cooler);
	~NetCooler();
	virtual float	getActualTemperature();
	virtual void	setTemperature(float _temperature);
	virtual void	setOn(bool onoff);
	virtual bool	isOn();
};

} // namespace net
} // namespace camera
} // namespace astro

#endif /* _NetCooler_h */
