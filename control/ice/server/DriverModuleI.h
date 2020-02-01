/*
 * DriverModuleI.h -- servant for driver modules
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DriverModuleI_h
#define _DriverModuleI_h

#include <device.h>
#include <AstroLoader.h>
#include "StatisticsI.h"

namespace snowstar {

class DriverModuleI : virtual public DriverModule, public StatisticsI {
	astro::module::ModulePtr	_module;
public:
	DriverModuleI(astro::module::ModulePtr module);
	virtual ~DriverModuleI();
	virtual std::string	getName(const Ice::Current& current);
	virtual std::string	getVersion(const Ice::Current& current);
	virtual bool	hasLocator(const Ice::Current& current);
	virtual DeviceLocatorPrx	getDeviceLocator(const Ice::Current& current);
};

} // namespace snowstar

#endif /* _DriverModuleI_h */
