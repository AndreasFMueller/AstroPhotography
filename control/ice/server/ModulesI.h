/*
 * ModulesI.h -- Modules access servant
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ModulesI_h
#define _ModulesI_h

#include <device.h>
#include <AstroLoader.h>
#include "StatisticsI.h"

namespace snowstar {

class ModulesI : virtual public Modules, public StatisticsI {
	astro::module::ModuleRepositoryPtr	_repository;
public:
	ModulesI();
	virtual ~ModulesI();
	virtual ModuleNameList	getModuleNames(const Ice::Current& current);
	virtual int	numberOfModules(const Ice::Current& current);
	virtual DriverModulePrx	getModule(const std::string& modulename,
			const Ice::Current& current);
};

} // namespace snowstar

#endif /* _ModulesI_h */
