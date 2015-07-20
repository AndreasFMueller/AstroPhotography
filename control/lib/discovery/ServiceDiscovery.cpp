/*
 * ServiceDiscovery.cpp -- classes to encapsulate dns service discover
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <ServiceDiscovery.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <stdexcept>
#include <algorithm>

#ifdef USE_SD_AVAHI
#include <AvahiDiscovery.h>
#endif /* USE_SD_AVAHI */

#if USE_SD_BONJOUR
#include <BonjourDiscovery.h>
#endif /* USE_SD_BONJOUR */

namespace astro {
namespace discover {

/**
 * \brief Auxiliary class to look for a service of a given name
 */
class HasNamePredicate {
	std::string	name;
public:
	HasNamePredicate(const std::string& n) : name(n) { }
	bool	operator()(const ServiceKey& key) const {
		return key.name() == name;
	}
};

ServiceDiscovery::ServiceDiscovery() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a service discovery object");
}

/**
 * \brief Destructor for a service discovery object
 *
 * The derived classes may need their own thread to run in, in those
 * cases the destructor has to take care of cancelling the thread
 */
ServiceDiscovery::~ServiceDiscovery() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy the service discovery object");
}

/**
 * \brief Factory method to create a service implementation
 *
 * This method creates a service discovery instance suitable for the
 * plattform.
 */
ServiceDiscoveryPtr	ServiceDiscovery::get() {
	// if we are on linux, we should create an instance of the
	// AvahiDiscovery class
#ifdef USE_SD_AVAHI
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"creating Avahi based service discovery");
	return ServiceDiscoveryPtr(new AvahiDiscovery());
#endif /* USE_SD_AVAHI */

	// on the Mac, we us an implementation that uses Apples Bonjour
	// implementation
#ifdef USE_SD_BONJOUR
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"creating Bonjour based service discovery");
	return ServiceDiscoveryPtr(new BonjourDiscovery());
#endif /* USE_SD_BONJOUR */
}

/**
 * \brief check whether the service name is already known
 */
bool	ServiceDiscovery::has(const std::string& name) {
	std::unique_lock<std::recursive_mutex>	lock(servicelock);
	ServiceKeySet::iterator	i
		= find_if(servicekeys.begin(), servicekeys.end(),
			HasNamePredicate(name));
	return (i != servicekeys.end());
}

bool	ServiceDiscovery::has(const ServiceKey& key) {
	ServiceKeySet::iterator	i;
	for (i = servicekeys.begin(); i != servicekeys.end(); i++) {
		if (key == *i) {
			return true;
		}
	}
	return false;
}

/**
 * \brief wait for a name to arrive
 */
ServiceKey	ServiceDiscovery::waitfor(const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for '%s'", name.c_str());
	std::unique_lock<std::recursive_mutex>	lock(servicelock);
	while (!has(name)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "name not found, waiting");
		servicecondition.wait(lock);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "condition called");
	}
	return find(name);
}

/**
 * \brief find a name in the list of available services, and resolve it
 */
ServiceKey	ServiceDiscovery::find(const std::string& name) {
	std::unique_lock<std::recursive_mutex>	lock(servicelock);
	ServiceKeySet::iterator	i
		= find_if(servicekeys.begin(), servicekeys.end(),
			HasNamePredicate(name));
	if (i == servicekeys.end()) {
		throw std::runtime_error("service not found");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %s", i->toString().c_str());
	return *i;
}

/**
 * \brief Add a service to the services set
 */
void	ServiceDiscovery::add(const ServiceKey& key) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add new key: %s",
		key.toString().c_str());
	std::unique_lock<std::recursive_mutex>	lock(servicelock);
	remove(key);
	servicekeys.insert(key);
	servicecondition.notify_all();
}

/**
 * \brief Remove a service from the services set
 */
void	ServiceDiscovery::remove(const ServiceKey& key) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove key: %s",
		key.toString().c_str());
	std::unique_lock<std::recursive_mutex>	lock(servicelock);
	ServiceKeySet::iterator	i = servicekeys.find(key);
	if (i != servicekeys.end()) {
		servicekeys.erase(i);
	}
}

/**
 * \brief Auxiliary class to display a list of services
 */
class ServiceDisplay {
	std::ostream&	_out;
public:
	ServiceDisplay(std::ostream& out) : _out(out) { }
	void	operator()(const ServiceObject& so) {
		_out << so.toString() << std::endl;
	}
};

class ServiceKeyDisplay {
	std::ostream&	_out;
public:
	ServiceKeyDisplay(std::ostream& out) : _out(out) { }
	void	operator()(const ServiceKey& key) {
		_out << key.toString() << std::endl;
	}
};

std::ostream&	operator<<(std::ostream& out,
			const ServiceDiscovery::ServiceKeySet& services) {
	for_each(services.begin(), services.end(), ServiceKeyDisplay(out));
	return out;
}

} // namespace discover
} // namespace astro
