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
void	callback_adapter(proxy& /* p */,
	const astro::callback::CallbackDataPtr /* d */) {
	std::string	msg
		= std::string("specialization for callback_adapter needed: ")
			+ std::string(typeid(proxy).name());;
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
public:
	void	registerCallback(const Ice::Identity& identity,
			const Ice::Current& current);
	void	unregisterCallback(const Ice::Identity& identity,
			const Ice::Current& current);
	void	cleanup(const std::list<Ice::Identity>& todelete);
	void	clear();
	astro::callback::CallbackDataPtr	operator()(astro::callback::CallbackDataPtr data);
	void	stop();
};

/**
 * \brief Register a callback with the callback object
 */
template<typename proxy>
void	SnowCallback<proxy>::registerCallback(const Ice::Identity& identity,
		const Ice::Current& current) {
	proxy	callback = proxy::uncheckedCast(
			current.con->createProxy(identity));
	callbacks.insert(std::make_pair(identity, callback));
}

/**
 * \brief Unregister a callback with the callback object
 */
template<typename proxy>
void	SnowCallback<proxy>::unregisterCallback(const Ice::Identity& identity,
		const Ice::Current& /* current */) {
	callbacks.erase(identity);
}

/**
 * \brief Cleanup routine to remove
 */
template<typename proxy>
void	SnowCallback<proxy>::cleanup(const std::list<Ice::Identity>& todelete) {
	for (auto ptr = todelete.begin(); ptr != todelete.end(); ptr++) {
		callbacks.erase(*ptr);
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
	// the todelete array is used to keep track of all the identities
	// that have failed
	std::list<Ice::Identity>	todelete;

	// go through all callbacks, and try to send them the data
	for (auto ptr = callbacks.begin(); ptr != callbacks.end(); ptr++) {
		try {
			callback_adapter<proxy>(ptr->second, data);
		} catch (...) {
			// callback has failed, keep its identity in order to
			// delete it later
			todelete.push_back(ptr->first);
		}
	}

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
