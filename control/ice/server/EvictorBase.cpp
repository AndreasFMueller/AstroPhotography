/*
 * EvictorBase.cpp  -- implementation of the EvictorBase class
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <EvictorBase.h>

namespace snowstar {

EvictorBase::EvictorBase(int size) : _size(size) {
}

EvictorBase::~EvictorBase() {
}

/**
 * \brief Locate the Servant
 */
Ice::ObjectPtr	EvictorBase::locate(const Ice::Current& current,
			Ice::LocalObjectPtr& cookie) {
	IceUtil::Mutex::Lock lock(_mutex);

	// Check if we have a servant in the map already.
	EvictorEntryPtr entry;
	EvictorMap::iterator i = _map.find(current.id);
	if (i != _map.end()) {
		// Got an entry already, dequeue the entry from
		// its current position.
		entry = i->second;
		_queue.erase(entry->queuePos);
	} else {
		//
		// We do not have an entry. Ask the derived class to
		// instantiate a servant and add a new entry to the map.
		//
		entry = new EvictorEntry;
		entry->servant = add(current, entry->userCookie); // Down-call
		if (!entry->servant) {
			return 0;
		}
		entry->useCount = 0;
		i = _map.insert(std::make_pair(current.id, entry)).first;
	}

	// Increment the use count of the servant and enqueue
	// the entry at the front, so we get LRU order.
	++(entry->useCount);
	entry->queuePos = _queue.insert(_queue.begin(), i);

	cookie = entry;

	return entry->servant;
}

/**
 * \brief Cleanup the servant after the call
 */
void	EvictorBase::finished(const Ice::Current& /* current */,
		const Ice::ObjectPtr& /* object */,
		const Ice::LocalObjectPtr& cookie) {
	IceUtil::Mutex::Lock lock(_mutex);

	EvictorEntryPtr entry = EvictorEntryPtr::dynamicCast(cookie);

	// Decrement use count and check if
	// there is something to evict.
	--(entry->useCount);
	evictServants();
}

/**
 * \brief Deactivate the servant locator
 *
 * This method evicts all servants
 */
void	EvictorBase::deactivate(const std::string& /* category */) {
	IceUtil::Mutex::Lock lock(_mutex);

	_size = 0;
	evictServants();
}

/**
 * \brief Evict LRU servants
 */
void	EvictorBase::evictServants() {
	//
	// If the evictor queue has grown larger than the limit,
	// look at the excess elements to see whether any of them
	// can be evicted.
	//
	EvictorQueue::reverse_iterator p = _queue.rbegin();
	int excessEntries = static_cast<int>(_map.size() - _size);

	for (int i = 0; i < excessEntries; ++i) {
		EvictorMap::iterator mapPos = *p;
		if (mapPos->second->useCount == 0) {
			// evict the entry if it is not currently in use
			evict(mapPos->second->servant,
				mapPos->second->userCookie);
			p = EvictorQueue::reverse_iterator(_queue.erase(
						mapPos->second->queuePos));
			_map.erase(mapPos);
		} else {
			++p;
		}
	}
}

} // namespace snowstar
