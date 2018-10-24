/*
 * RepositoryLocator.h -- locator for guider servants
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _RepositoryLocator_h
#define _RepositoryLocator_h

#include <Ice/Ice.h>
#include <repository.h>

namespace snowstar {

class RepositoryLocator : public Ice::ServantLocator {
	typedef std::map<std::string, Ice::ObjectPtr>	repositorymap;
	repositorymap	repositories;
public:
	RepositoryLocator();
	virtual ~RepositoryLocator() { }

	void	add(const std::string& name, Ice::ObjectPtr repositoryptr);

	virtual Ice::ObjectPtr	locate(const Ice::Current& current,
			Ice::LocalObjectPtr& cookie);

	virtual void	finished(const Ice::Current& current,
				const Ice::ObjectPtr& servant,
				const Ice::LocalObjectPtr& cookie);

	virtual void	deactivate(const std::string& category);
};

} // namespace snowstar

#endif /* _RepositoryLocator_h */
