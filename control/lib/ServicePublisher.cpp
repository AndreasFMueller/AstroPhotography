/*
 * ServicePublisher.cpp -- classes to encapsulate dns service publishing
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

ServicePublisher::ServicePublisher(const std::string& servername, int port)
	: _servername(servername), _port(port) {
	if (0 == servername.size()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "servername may not be empty");
		throw std::range_error("servername may not empty");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"create a service publishing object named %s:%d",
		servername.c_str(), port);
}

/**
 * \brief Destructor for a service discovery object
 *
 * The derived classes may need their own thread to run in, in those
 * cases the destructor has to take care of cancelling the thread
 */
ServicePublisher::~ServicePublisher() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy the service publishing object");
}

/**
 * \brief Publish a service object
 */
void	ServicePublisher::add(ServiceObject::service_type type) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add %s",
		ServiceObject::type_name(type).c_str());
	published.insert(type);
}

void	ServicePublisher::publish() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "publish now");
}

/**
 * \brief Predicate class to be used in algorithms
 */
class HasTypePredicate {
	ServiceObject::service_type	_type;
public:
	HasTypePredicate(ServiceObject::service_type type) : _type(type) { }
	bool	operator()(ServiceObject::service_type type) {
		return _type == type;
	}
};

/**
 * \brief Revoke a service
 */
void	ServicePublisher::revoke(ServiceObject::service_type type) {
	ServiceTypeSet::iterator	i
		= find_if(published.begin(), published.end(),
			HasTypePredicate(type));
	if (i != published.end()) {
		published.erase(i);
	}
}

/**
 * \brief Factory method to create a service implementation
 *
 * This method creates a service discovery instance suitable for the
 * plattform.
 */
ServicePublisherPtr	ServicePublisher::get(const std::string& servername,
				int port) {
	// if we are on linux, we should create an instance of the
	// AvahiPublisher class
#ifdef USE_SD_AVAHI
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"creating Avahi based service discovery");
	return ServicePublisherPtr(new AvahiPublisher(servername, port));
#endif /* USE_SD_AVAHI */

	// on the Mac, we us an implementation that uses Apples Bonjour
	// implementation
#ifdef USE_SD_BONJOUR
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"creating Bonjour based service discovery");
	return ServicePublisherPtr(new BonjourPublisher(servername, port));
#endif /* USE_SD_BONJOUR */
}

} // namespace discover
} // namespace astro
