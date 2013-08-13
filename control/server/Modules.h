/*
 * Modules.h -- Modules servant implementation
 *
 * (c) Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Modules_h
#define _Modules_h

#include <../idl/device.hh>
#include <AstroLoader.h>

namespace Astro {

class Modules_impl : public POA_Astro::Modules {
	astro::module::Repository	repository;
	typedef	std::map<std::string, astro::module::ModulePtr>	modulemap_t;
	modulemap_t	modulemap;
public:
	inline Modules_impl() { }
	virtual ~Modules_impl() { }
	virtual ::CORBA::Long	numberOfModules();
	virtual Astro::Modules::ModuleNameSequence	*getModuleNames();
	virtual Astro::_objref_DriverModule 	*getModule(const char*);
};

} // namespace Astro

#endif /* _Modules_h */
