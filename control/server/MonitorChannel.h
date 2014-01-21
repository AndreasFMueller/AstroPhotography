/*
 * MonitorChannel.h -- MonitorChannel template to distribute monitor updates
 *                     to all interested recipients
 *
 * (c) 2014 Prof Dr Andreas Mueller, 
 */
#include <guider.hh>
#include <AstroGuiding.h>
#include <map>

namespace Astro {

/**
 * \brief Template for subscribe/unsubscribe mechanism for callbacks
 *
 * \param monitorinterface	The interface type of the monitor interface
 *				that should be served with updates
 * \param argtype		The argument type for the updates
 *
 */
template<typename monitorinterface, typename argtype>
class MonitorChannel {
	pthread_mutex_t	mutex;
	typedef std::map< ::CORBA::Long, typename monitorinterface::_var_type>	monitormap_t;
	monitormap_t	monitors;
public:
	MonitorChannel();
	~MonitorChannel();
	void	update(const argtype& data);
	void	stop();
	::CORBA::Long	subscribe(typename monitorinterface::_ptr_type);
	void	unsubscribe(::CORBA::Long id);
};

/**
 * \brief Create a new monitor channel
 *
 * The constructor initializes the pthread mutex that is used to protect
 * the monitor map.
 */
template<typename monitorinterface, typename argtype>
MonitorChannel<monitorinterface, argtype>::MonitorChannel() {
	// create a recursive mutex to protect the monitor map
	pthread_mutexattr_t	mattr;
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutex, &mattr);
}

/**
 * \brief Destroy a monitor channel
 */
template<typename monitorinterface, typename argtype>
MonitorChannel<monitorinterface, argtype>::~MonitorChannel() {
	pthread_mutex_destroy(&mutex);
}

/**
 * \brief subscribe a monitor to the channel
 *
 * This adds the monitor reference to the map with a new id, which is returned.
 * \param	The monitor reference to subscribe to the channel.
 * \return	The id of the monitor interface. To be used by the monitor 
 *		client to unregister the interface when it is no longer
 *		interested in updates.
 */
template<typename monitorinterface, typename argtype>
::CORBA::Long	MonitorChannel<monitorinterface, argtype>::subscribe(
			typename monitorinterface::_ptr_type monitor) {
	// lock the mutex to protect the monitors
	pthread_mutex_lock(&mutex);

	// first find a new monitorid
	::CORBA::Long	monitorid = 0;
	typename monitormap_t::iterator	i;
	for (i = monitors.begin(); i != monitors.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "checking id %d", i->first);
		if (i->first >= monitorid) {
			monitorid = i->first + 1;
		}
	}

	// create a copy of the reference, i.e. increment the reference
	// counter so that it survives this call
	typename monitorinterface::_var_type	monitorvar
		= monitorinterface::_duplicate(monitor);

	// add the monitor to the map. The key is the monitor id, so that
	// we can easily find it in the unsubscribe method
	monitors.insert(std::make_pair(monitorid, monitorvar));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracking monitor registered as %ld",
		monitorid);

	// release the lock
	pthread_mutex_unlock(&mutex);
	return monitorid;
}

/**
 * \brief unsubscribe a monitor from the monitor channel
 *
 * If there is a monitor for the id specified as argument, it is removed
 * from the map.
 * \param monitorid	id of the monitor to remove from the 
 */
template<typename monitorinterface, typename argtype>
void	MonitorChannel<monitorinterface, argtype>::unsubscribe(::CORBA::Long id) {
	// lock the mutex to protect the monitors
	pthread_mutex_lock(&mutex);

	// find the object
	if (monitors.find(id) != monitors.end()) {
		try {
			monitors.erase(id);
		} catch (...) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"failed to remove image monitor %ld", id);
		}
	} else {
		debug(LOG_ERR, DEBUG_LOG, 0, "image monitor %ld does not exist",
			id);
		throw CORBA::OBJECT_NOT_EXIST();
	}

	// release the lock
	pthread_mutex_unlock(&mutex);
}

/**
 * \brief update a monitor, send data to all subscribed monitors
 *
 * As a side effect, this method also removes all monitors that fail.
 */
template<typename monitorinterface, typename argtype>
void	MonitorChannel<monitorinterface, argtype>::update(const argtype& data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracking image update received");

	// lock the mutex to protect the map
	pthread_mutex_lock(&mutex);

	// build a list of bad monitors which we will later remove from the map
	std::vector< ::CORBA::Long>	badmonitors;

	// send the trackinginfo update to all monitors
	typename monitormap_t::iterator	i;
	for (i = monitors.begin(); i != monitors.end(); i++) {
		try {
			i->second->update(data);
		} catch (...) {
			badmonitors.push_back(i->first);
		}
	}

	// remove all monitors that are bad
	std::vector< ::CORBA::Long>::iterator	j;
	for (j = badmonitors.begin(); j != badmonitors.end(); j++) {
		try {
			monitors.erase(*j);
		} catch (...) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"error while removing imagemonitor %ld", *j);
		}
	}

	// release the lock
	pthread_mutex_unlock(&mutex);
}

/**
 * \brief Inform the clients that guiding has stopped
 */
template<typename monitorinterface, typename argtype>
void	MonitorChannel<monitorinterface, argtype>::stop() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracking update stop received");

	// lock the mutex to protect the 
	pthread_mutex_lock(&mutex);

	// prepare a list of monitors
	std::vector< ::CORBA::Long>	badmonitors;

	// send the trackinginfo update to all monitors
	typename monitormap_t::iterator	i;
	for (i = monitors.begin(); i != monitors.end(); i++) {
		try {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "sending stop to %d",
				i->first);
			i->second->stop();
		} catch (...) {
			debug(LOG_ERR, DEBUG_LOG, 0, "error while updating %d",
				i->first);
			badmonitors.push_back(i->first);
		}
	}

	// remove all monitors that are bad
	std::vector< ::CORBA::Long>::iterator	j;
	for (j = badmonitors.begin(); j != badmonitors.end(); j++) {
		try {
			monitors.erase(*j);
		} catch (...) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"error while removing monitor %ld", *j);
		}
	}

	// release the lock
	pthread_mutex_unlock(&mutex);
}

} // namespace Astro
