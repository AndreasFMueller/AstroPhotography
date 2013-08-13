/*
 * DriverModule.h -- Driver Module C++ interface
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DriverModule_h
#define _DriverModule_h

#include <../idl/device.hh>
#include <AstroLoader.h>

namespace Astro {

class DriverModule_impl : public POA_Astro::DriverModule {
	astro::module::ModulePtr	_module;
public:
	inline DriverModule_impl(astro::module::ModulePtr module)
		: _module(module) { }
	virtual ~DriverModule_impl() { }
	virtual char	*getName();
};

} // namespace Astro

#endif /* _DriverModule_h */
