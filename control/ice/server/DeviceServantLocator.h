/*
 * DeviceServantLocator.h -- definition of a servant locator
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DeviceServantLocator_h
#define _DeviceServantLocator_h

#include <Ice/Ice.h>
#include <AstroLoader.h>
#include <ImageDirectory.h>

namespace snowstar {

class DeviceServantLocator : public Ice::ServantLocator {
	astro::module::Repository	_repository;
	astro::image::ImageDirectory&	_imagedirectory;
	typedef std::map<std::string, Ice::ObjectPtr>	devicemap;
	devicemap	devices;
public:
	DeviceServantLocator(astro::module::Repository& repository,
		astro::image::ImageDirectory& imagedirectory);

	virtual Ice::ObjectPtr	locate(const Ice::Current& current,
			Ice::LocalObjectPtr& cookie);

	virtual void	finished(const Ice::Current& current,
				const Ice::ObjectPtr& servant,
				const Ice::LocalObjectPtr& cookie);

	virtual void	deactivate(const std::string& category);
};

} // namespace snowstar

#endif /* _DeviceServantLocator_h */
