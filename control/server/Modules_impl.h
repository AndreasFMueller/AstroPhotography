/*
 * Modules_impl.h -- Modules servant implementation
 *
 * (c) Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Modules_impl_h
#define _Modules_impl_h

#include <device.hh>
#include <AstroLoader.h>

namespace Astro {

/**
 * \brief Modules servant definition
 */
class Modules_impl : public POA_Astro::Modules {
	astro::module::Repository	repository;
	typedef	std::map<std::string, astro::module::ModulePtr>	modulemap_t;
	modulemap_t	modulemap;
	std::vector<std::string>	modulenames();
public:
	inline Modules_impl() { }
	virtual ~Modules_impl() { }
	virtual ::CORBA::Long	numberOfModules();
	virtual Astro::Modules::ModuleNameSequence	*getModuleNames();
	virtual Astro::_objref_DriverModule 	*getModule(const char*);
};

} // namespace Astro

#endif /* _Modules_impl_h */
