/*
 * GuiderLocator.h -- locator for guider servants
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuiderLocator_h
#define _GuiderLocator_h

#include <Ice/Ice.h>

namespace snowstar {

class GuiderLocator : public Ice::ServantLocator {
	typedef std::map<std::string, Ice::ObjectPtr>	guidermap;
	guidermap	guiders;
public:
	GuiderLocator();

	bool	has(const std::string& gn);

	void	add(const std::string& name, Ice::ObjectPtr guiderptr);

	virtual Ice::ObjectPtr	locate(const Ice::Current& current,
			Ice::LocalObjectPtr& cookie);

	virtual void	finished(const Ice::Current& current,
				const Ice::ObjectPtr& servant,
				const Ice::LocalObjectPtr& cookie);

	virtual void	deactivate(const std::string& category);
};

} // namespace snowstar

#endif /* _GuiderLocator_h */
