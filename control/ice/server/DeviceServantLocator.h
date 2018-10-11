/*
 * DeviceServantLocator.h -- definition of a servant locator
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DeviceServantLocator_h
#define _DeviceServantLocator_h

#include <Ice/Ice.h>
#include <AstroLoader.h>

namespace snowstar {

class DeviceServantLocator : public Ice::ServantLocator {
	astro::module::ModuleRepositoryPtr	_repository;
	typedef std::map<std::string, Ice::ObjectPtr>	devicemap;
	devicemap	devices;
public:
	DeviceServantLocator(astro::module::ModuleRepositoryPtr repository);

	virtual Ice::ObjectPtr	locate(const Ice::Current& current,
			Ice::LocalObjectPtr& cookie);

	virtual void	finished(const Ice::Current& current,
				const Ice::ObjectPtr& servant,
				const Ice::LocalObjectPtr& cookie);

	virtual void	deactivate(const std::string& category);
};

} // namespace snowstar

#endif /* _DeviceServantLocator_h */
