/*
 * ServicePublisher.cpp -- classes to encapsulate dns service publishing
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <AstroDiscovery.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <stdexcept>
#include <algorithm>

#ifdef USE_SD_AVAHI
#include "AvahiDiscovery.h"
#endif /* USE_SD_AVAHI */

#if USE_SD_BONJOUR
#include "BonjourDiscovery.h"
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
	remove_published(_servername);
}

void	ServicePublisher::publish() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "publish now");
	add_published(_servername);
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

static std::recursive_mutex	published_mtx;

static std::set<std::string>	published_services;

void	ServicePublisher::add_published(const std::string& name) {
	std::unique_lock<std::recursive_mutex>	lock(published_mtx);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add published name: %s", name.c_str());
	published_services.insert(name);
}

void	ServicePublisher::remove_published(const std::string& name) {
	std::unique_lock<std::recursive_mutex>	lock(published_mtx);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "removing name '%s'", name.c_str());
	std::set<std::string>::iterator	i = published_services.find(name);
	if (i != published_services.end()) {
		published_services.erase(i);
	}
}

bool	ServicePublisher::ispublished(const std::string& name) {
	std::unique_lock<std::recursive_mutex>	lock(published_mtx);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "check whether '%s' is published",
		name.c_str());
	std::set<std::string>::iterator	i = published_services.find(name);
	return (i != published_services.end());
}

} // namespace discover
} // namespace astro
