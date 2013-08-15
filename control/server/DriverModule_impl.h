/*
 * DriverModule_impl.h -- Driver Module C++ interface
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DriverModule_impl_h
#define _DriverModule_impl_h

#include <../idl/device.hh>
#include <AstroLoader.h>

namespace Astro {

class DriverModule_impl : public POA_Astro::DriverModule {
	astro::module::ModulePtr	_module;
	astro::device::DeviceLocatorPtr	_devicelocator;
public:
	inline DriverModule_impl(astro::module::ModulePtr module)
		: _module(module) {
	}
	virtual ~DriverModule_impl() { }
	virtual char	*getName();
	virtual Descriptor	*getDescriptor();
	virtual DeviceLocator_ptr	getDeviceLocator();
};

} // namespace Astro

#endif /* _DriverModule_impl_h */
