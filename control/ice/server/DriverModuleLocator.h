/*
 * DriverModuleLocator.h -- locate a device module
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DriverModuleLocator_h
#define _DriverModuleLocator_h

#include <Ice/Ice.h>
#include <AstroLoader.h>

namespace snowstar {

class DriverModuleLocator : public Ice::ServantLocator {
	astro::module::Repository	_repository;
public:
	DriverModuleLocator(astro::module::Repository& repository);
	virtual ~DriverModuleLocator();
	virtual Ice::ObjectPtr	locate(const Ice::Current& current,
			Ice::LocalObjectPtr& cookie);
	virtual void	finished(const Ice::Current& current,
				const Ice::ObjectPtr& servant,
				const Ice::LocalObjectPtr& cookie);

	virtual void	deactivate(const std::string& category);
};

} // namespace snowstar

#endif /* _DriverModuleLocator_h */
