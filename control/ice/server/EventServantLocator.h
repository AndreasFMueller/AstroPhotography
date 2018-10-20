/*
 * EventServantLocator.h -- event servant locator class
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _EventServantLocator_h
#define _EventServantLocator_h

#include <Ice/Ice.h>

namespace snowstar {

class EventServantLocator : public Ice::ServantLocator {
public:
	EventServantLocator() { }
	virtual ~EventServantLocator() { }

	virtual Ice::ObjectPtr	locate(const Ice::Current& current,
			Ice::LocalObjectPtr& cookie);

	virtual void	finished(const Ice::Current& current,
				const Ice::ObjectPtr& servant,
				const Ice::LocalObjectPtr& cookie);

	virtual void	deactivate(const std::string& category);
};

} // namespace snowstar

#endif /* _EventServantLocator_h */
