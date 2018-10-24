/*
 * TaskLocator.h -- locate a task servant
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _TaskLocator_h
#define _TaskLocator_h

#include <Ice/Ice.h>
#include <AstroPersistence.h>

namespace snowstar {

class TaskLocator : public Ice::ServantLocator {
	astro::persistence::Database&	database;
public:
	TaskLocator(astro::persistence::Database& database);
	virtual ~TaskLocator() { }

	virtual Ice::ObjectPtr	locate(const Ice::Current& current,
			Ice::LocalObjectPtr& cookie);

	virtual void	finished(const Ice::Current& current,
				const Ice::ObjectPtr& servant,
				const Ice::LocalObjectPtr& cookie);

	virtual void	deactivate(const std::string& category);
};

} // namespace snowstar

#endif /* _TaskLocator_h */
