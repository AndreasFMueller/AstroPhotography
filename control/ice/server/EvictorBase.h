/*
 * EvictorBase.h -- evictor that removes least recently used servants
 *
 * This is based on the following best practice from the zeroc website:
 *
 * https://doc.zeroc.com/ice/3.6/best-practices/servant-evictors/implementing-a-servant-evictor-in-c++
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _EvictorBase_h
#define _EvictorBase_h

#include <Ice/Ice.h>

namespace snowstar {

class EvictorBase : public Ice::ServantLocator {

public:
	EvictorBase(int size = 10);
	virtual ~EvictorBase();
	virtual Ice::ObjectPtr	locate(const Ice::Current& current,
					Ice::LocalObjectPtr& cookie);
	virtual void	finished(const Ice::Current& current,
				const Ice::ObjectPtr& object,
				const Ice::LocalObjectPtr& cookie);
	virtual void	deactivate(const std::string& category);
protected:
	virtual Ice::ObjectPtr	add(const Ice::Current& current,
					Ice::LocalObjectPtr& cookie) = 0;
	virtual void	evict(const Ice::ObjectPtr& object,
				const Ice::LocalObjectPtr& cookie) = 0;
private:
	struct EvictorEntry;
	typedef IceUtil::Handle<EvictorEntry>	EvictorEntryPtr;
	typedef std::map<Ice::Identity, EvictorEntryPtr>	EvictorMap;
	typedef std::list<EvictorMap::iterator>	EvictorQueue;

	struct EvictorEntry : public Ice::LocalObject {
		Ice::ObjectPtr		servant;
		Ice::LocalObjectPtr	userCookie;
		EvictorQueue::iterator	queuePos;
		int	useCount;
	};

	EvictorMap	_map;
	EvictorQueue	_queue;
	Ice::Int	_size;
	IceUtil::Mutex	_mutex;

	void	evictServants();
};

typedef IceUtil::Handle<EvictorBase>	EvictorBasePtr;

} // namespace snowstar

#endif /* _EvictorBase_h */
