/*
 * ImageLocator.h -- ImageLocator declarations
 *
 * (c) 2014 Prof DR Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImageLocator_h
#define _ImageLocator_h

#include <Ice/Ice.h>
#include <map>
#include <ImageI.h>
#include <mutex>

namespace snowstar {

class ImageLocator : public Ice::ServantLocator {
	typedef std::map<std::string, Ice::ObjectPtr>	imagemap;
	imagemap	images;
	std::mutex	imagemutex;
public:
	ImageLocator();

	virtual Ice::ObjectPtr	locate(const Ice::Current& current,
			Ice::LocalObjectPtr& cookie);

	virtual void	finished(const Ice::Current& current,
				const Ice::ObjectPtr& servant,
				const Ice::LocalObjectPtr& cookie);

	virtual void	deactivate(const std::string& category);

	void	expire();
};

} // namespace snowstar

#endif /* _ImageLocator_h */
