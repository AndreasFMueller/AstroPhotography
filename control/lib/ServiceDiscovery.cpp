/*
 * ServiceDiscover.cpp -- classes to encapsulate dns service discover
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

//////////////////////////////////////////////////////////////////////
// Implementation of the ServiceObject class
//////////////////////////////////////////////////////////////////////

/**
 * \brief Create a SerivceObject from name and port
 */
ServiceObject::ServiceObject(const std::string& name, int port,
	const std::string host)
	: _name(name), _port(port), _host(host) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new service %s", toString().c_str());
}

/**
 * \brief Convert the service object into a string representation
 */
std::string	ServiceObject::toString() const {
	return stringprintf("%s:%d@%s", name().c_str(), port(), host().c_str());
}

/**
 * \brief Comparision operator for Service objects
 */
bool	ServiceObject::operator<(const ServiceObject& other) const {
	return (_name < other._name);
}

//////////////////////////////////////////////////////////////////////
// ServiceDiscovery interface class implementation
//////////////////////////////////////////////////////////////////////

ServiceDiscovery::ServiceDiscovery(service_type t) : _type(t) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a service discovery object");
	dnssd_type = std::string("_") + type_name(t) + std::string("._tcp");
	dnssd_type = std::string("_ssh._tcp");
}

/**
 * \brief Convert the service type into a descriptive string
 */
std::string	ServiceDiscovery::type_name(service_type t) {
	switch (t) {
	case INSTRUMENT:
		return std::string("instrument");
	case TASKS:
		return std::string("tasks");
	case GUIDING:
		return std::string("guiding");
	case IMAGES:
		return std::string("images");
	}
	throw std::range_error("unknown service type");
}

/**
 * \brief Convert a type name into a type code
 */
ServiceDiscovery::service_type	ServiceDiscovery::type_name(const std::string& n) {
	if (n == "instrument") {
		return INSTRUMENT;
	}
	if (n == "instrument") {
		return TASKS;
	}
	if (n == "instrument") {
		return GUIDING;
	}
	if (n == "instrument") {
		return IMAGES;
	}
	throw std::runtime_error("convert service type name to code");
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
 * \brief Publish a service object
 */
void	ServiceDiscovery::publish(const ServiceObject& object) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "publish %s", object.toString().c_str());
	// XXX no implementation
	published.insert(object);
}

/**
 * \brief Factory method to create a service implementation
 *
 * This method creates a service discovery instance suitable for the
 * plattform.
 */
ServiceDiscoveryPtr	ServiceDiscovery::get(service_type t) {
	// if we are on linux, we should create an instance of the
	// AvahiDiscovery class
#ifdef USE_SD_AVAHI
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"creating Avahi based service discovery");
	return ServiceDiscoveryPtr(new AvahiDiscovery(t));
#endif /* USE_SD_AVAHI */

	// on the Mac, we us an implementation that uses Apples Bonjour
	// implementation
#ifdef USE_SD_BONJOUR
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"creating Bonjour based service discovery");
	return ServiceDiscoveryPtr(new BonjourDiscovery(t));
#endif /* USE_SD_BONJOUR */
}

const std::set<ServiceObject>&	ServiceDiscovery::resolve() const {
	return services;
}

void	ServiceDiscovery::add(const ServiceObject& so) {
	services.insert(so);
}

class HasNamePredicate {
	std::string	name;
public:
	HasNamePredicate(const std::string& n) : name(n) { }
	bool	operator()(const ServiceObject& so) const {
		return so.name() == name;
	}
};

void	ServiceDiscovery::remove(const std::string& name) {
	ServiceSet::iterator	i = find_if(services.begin(), services.end(),
					HasNamePredicate(name));
	if (i != services.end()) {
		services.erase(i);
	}
}

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
