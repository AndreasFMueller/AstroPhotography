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

#if USE_SD_BONOUR
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
	bool	operator()(const ServiceObject& so) const {
		return so.name() == name;
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

const std::set<ServiceObject>&	ServiceDiscovery::resolve() const {
	return services;
}

/**
 * \brief Add a service to the services set
 */
void	ServiceDiscovery::add(const ServiceObject& so) {
	services.insert(so);
}

/**
 * \brief Remove a service from the services set
 */
void	ServiceDiscovery::remove(const std::string& name) {
	ServiceSet::iterator	i = find_if(services.begin(), services.end(),
					HasNamePredicate(name));
	if (i != services.end()) {
		services.erase(i);
	}
}

/**
 * \brief Find a service object
 */
const ServiceObject&	ServiceDiscovery::find(const std::string& name) const {
	ServiceSet::iterator	i = find_if(services.begin(), services.end(),
					HasNamePredicate(name));
	if (i != services.end()) {
		return *i;
	}
	throw std::runtime_error("service not found");
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

std::ostream&	operator<<(std::ostream& out,
			const ServiceDiscovery::ServiceSet& services) {
	for_each(services.begin(), services.end(), ServiceDisplay(out));
	return out;
}

} // namespace discover
} // namespace astro
