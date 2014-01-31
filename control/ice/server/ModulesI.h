/*
 * ModulesI.h -- module implementation for ICE server
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ModulesI_h
#define _ModulesI_h

#include <module.h>
#include <AstroLoader.h>

namespace snowstar {

class ModulesI : public Modules {
	astro::module::Repository	repository;
	typedef std::map<std::string, astro::module::ModulePtr>	modulemap_t;
	modulemap_t	modulemap;
	std::vector<std::string>	modulenames();
public:
	virtual int	numberOfModules(const Ice::Current& current);
	virtual ModuleNameSequence getModuleNames(const Ice::Current& current);
	virtual DriverModulePrx getModule(const std::string& modulename,
			const Ice::Current& current);
};

} // namespace snowstar

#endif /* _ModulesI_h */
