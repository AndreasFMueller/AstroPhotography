/*
 * ImageLocator.h -- ImageLocator declarations
 *
 * (c) 2014 Prof DR Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImageLocator_h
#define _ImageLocator_h

#include <Ice/Ice.h>
#include <ImageDirectory.h>
#include <map>
#include <ImageI.h>

namespace snowstar {

class ImageLocator : public Ice::ServantLocator {
	astro::image::ImageDirectory&	_imagedirectory;
	typedef std::map<std::string, Ice::ObjectPtr>	imagemap;
	imagemap	images;
public:
	ImageLocator(astro::image::ImageDirectory& imagedirectory);

	virtual Ice::ObjectPtr	locate(const Ice::Current& current,
			Ice::LocalObjectPtr& cookie);

	virtual void	finished(const Ice::Current& current,
				const Ice::ObjectPtr& servant,
				const Ice::LocalObjectPtr& cookie);

	virtual void	deactivate(const std::string& category);
};

} // namespace snowstar

#endif /* _ImageLocator_h */
