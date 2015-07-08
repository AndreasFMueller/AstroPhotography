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

ServiceObject	ServiceDiscovery::find(const std::string& name) {
	ServiceKeySet::iterator	i
		= find_if(servicekeys.begin(), servicekeys.end(),
			HasNamePredicate(name));
	if (i == servicekeys.end()) {
		throw std::runtime_error("service not found");
	}
	return find(*i);
}

/**
 * \brief Add a service to the services set
 */
void	ServiceDiscovery::add(const ServiceKey& key) {
	remove(key);
	servicekeys.insert(key);
}

/**
 * \brief Remove a service from the services set
 */
void	ServiceDiscovery::remove(const ServiceKey& key) {
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
