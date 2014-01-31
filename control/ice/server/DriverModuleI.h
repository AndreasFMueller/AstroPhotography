/*
 * DriverModuleI.h -- Driver Module ICE servant definition
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DriverModuleI_h
#define _DriverModuleI_h

#include <module.h>
#include <AstroLoader.h>

namespace snowstar {

class DriverModuleI : public DriverModule {
	astro::module::ModulePtr	_module;
public:
	DriverModuleI(astro::module::ModulePtr module)
		: _module(module) { }
	virtual ~DriverModuleI() { }
	std::string	getName(const ::Ice::Current& current);
	Descriptor	getDescriptor(const ::Ice::Current& current);
	DeviceLocatorPtr	getDeviceLocator(const ::Ice::Current& current);
};

} // namespace snowstar

#endif /* _DriverModuleI_h */
