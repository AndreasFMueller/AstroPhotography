/*
 * InstrumentLocator.h -- InstrumentLocator declarations
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _InstrumentLocator_h
#define _InstrumentLocator_h

#include <Ice/Ice.h>
#include <instruments.h>
#include <map>

namespace snowstar {

class InstrumentLocator : public Ice::ServantLocator {
	typedef std::map<std::string, Ice::ObjectPtr>	instrumentmap;
	instrumentmap	instruments;
public:
	InstrumentLocator();
	virtual ~InstrumentLocator() { }

	virtual Ice::ObjectPtr	locate(const Ice::Current& current,
					Ice::LocalObjectPtr& cookie);
	virtual void	finished(const Ice::Current& current,
				const Ice::ObjectPtr& servant,
				const Ice::LocalObjectPtr& cookie);

	virtual void	deactivate(const std::string& category);
};


} // namespace snowstar

#endif /* _InstrumentLocator_h */
