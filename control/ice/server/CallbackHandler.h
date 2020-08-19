/*
 * CallbackHandler.h -- classes to handle callbacks registered from a client
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CallbackHandler_h
#define _CallbackHandler_h

#include <AstroCallback.h>
#include <typeinfo>
#include <Ice/Connection.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <mutex>

namespace snowstar {

/**
 * \brief Callback adapter function
 *
 * The classes actually implemenenting the callbacks will need to specialize
 * this template for their purposes. For each callback interface they use
 * they will have to provide an implementation that converts the data 
 * in argument d, converts it to the arguments used by the callback interface,
 * and sends it to the proxy.
 */
template<typename proxy>
void	callback_adapter(proxy /* p */,
	const astro::callback::CallbackDataPtr /* d */) {
	std::string	msg
		= std::string("specialization for callback_adapter needed: ")
			+ std::string(typeid(proxy).name());;
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Callback template for snowstar server
 *
 * This template contains all the magic of maintining a registry of proxies
 * to which to send the callback data. It does not care about the mechanics
 * of actually calling the callback interface from the callback data provided,
 * this is the task of the adapter
 */
template<typename proxy>
class SnowCallback {
	std::map<Ice::Identity, proxy>	callbacks;
	std::recursive_mutex	_mutex;
public:
	void	registerCallback(const Ice::Identity& identity,
			const Ice::Current& current);
	void	unregisterCallback(const Ice::Identity& identity,
			const Ice::Current& current);
	void	cleanup(const std::list<Ice::Identity>& todelete);
	void	clear();
	virtual astro::callback::CallbackDataPtr	operator()(
			astro::callback::CallbackDataPtr data);
	void	stop();
	size_t	size() const { return callbacks.size(); }
};

/**
 * \brief Register a callback with the callback object
 *
 * The callbacks must use oneway calls to prevent deadlocks: for this we use
 * the ice_oneway method to create a oneway proxy from the identity. We then
 * only keep the oneway proxy in the map.
 */
template<typename proxy>
void	SnowCallback<proxy>::registerCallback(const Ice::Identity& identity,
		const Ice::Current& current) {
	Ice::ObjectPrx	oneway
		= current.con->createProxy(identity)->ice_oneway();
	proxy	callback = proxy::uncheckedCast(oneway);
	callbacks.insert(std::make_pair(identity, callback));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s callback %s installed, %d clients", 
		astro::demangle(typeid(proxy).name()).c_str(),
		identity.name.c_str(), size());
}

/**
 * \brief Unregister a callback with the callback object
 */
template<typename proxy>
void	SnowCallback<proxy>::unregisterCallback(const Ice::Identity& identity,
		const Ice::Current& /* current */) {
	{
		std::unique_lock<std::recursive_mutex>	lock(_mutex);
		auto	i = callbacks.find(identity);
		if (i != callbacks.end()) {
			callbacks.erase(i);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s callback uninstalled, %d clients", 
		astro::demangle(typeid(proxy).name()).c_str(), size());
}

/**
 * \brief Cleanup routine to remove a list of identities
 */
template<typename proxy>
void	SnowCallback<proxy>::cleanup(const std::list<Ice::Identity>& todelete) {
	for (auto ptr = todelete.begin(); ptr != todelete.end(); ptr++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "removing %s",
			ptr->name.c_str());
		std::unique_lock<std::recursive_mutex>	lock(_mutex);
		auto	i = callbacks.find(*ptr);
		if (i != callbacks.end()) {
			callbacks.erase(i);
		}
	}
}

/**
 * \brief Remove all callbacks
 */
template<typename proxy>
void	SnowCallback<proxy>::clear() {
	callbacks.clear();
}

/**
 * \brief perform a callback call
 *
 * This method goes through all the registered callbacks, and sends the
 * data provided to them, using the callback adapter template method (for
 * which the server also has to provide an implementation)
 */
template<typename proxy>
astro::callback::CallbackDataPtr	SnowCallback<proxy>::operator()(
	astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"%s callback data received, %d clients, proxy=%s",
		astro::demangle(typeid(proxy).name()).c_str(),
		size(), typeid(proxy).name());
	// the todelete array is used to keep track of all the identities
	// that have failed
	std::list<Ice::Identity>	todelete;

	// go through all callbacks, and try to send them the data
	{
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	for (auto ptr = callbacks.begin(); ptr != callbacks.end(); ptr++) {
		try {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"calling callback_adapter<proxy> %s",
				ptr->first.name.c_str());
			callback_adapter<proxy>(ptr->second, data);
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "callback failed: %s",
				x.what());
			debug(LOG_DEBUG, DEBUG_LOG, 0, "removing %s",
				astro::demangle(ptr->first.name).c_str());
			// callback has failed, keep its identity in order to
			// delete it later
			todelete.push_back(ptr->first);
		}
	}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d callbacks called, %d to delete",
		size(), todelete.size());

	// now erase all identities for which the callback has failed, we
	// automatically unregister them
	cleanup(todelete);

	// return the original data unchanged
	return data;
}

/**
 * \brief Send the stop signal to all callbacks
 */
template<typename proxy>
void	SnowCallback<proxy>::stop() {
	std::list<Ice::Identity>	todelete;
	for (auto ptr = callbacks.begin(); ptr != callbacks.end(); ptr++) {
		try {
			ptr->second->stop();
		} catch (...) {
			todelete.push_back(ptr->first);
		}
	}
	cleanup(todelete);
}

} // namespace snowstar

#endif /* _CallbackHandler_h */
