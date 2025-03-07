/*
 * AvahiDiscovery.cpp -- Avahi-based service discovery implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include "AvahiDiscovery.h"
#include <AstroDebug.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-client/publish.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>

namespace astro {
namespace discover {

/**
 * \brief Constructor for the AvahiDiscovery object
 *
 * This constructor also creates the thread and launches it on the avahi_main
 * function, which is just a redirection to the main method of the 
 * AvahiDiscovery object.
 */
AvahiDiscovery::AvahiDiscovery() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create AvahiDiscovery object");
}

/**
 * \brief Destroy the avahi discovery object
 *
 * This destrutctor must cancel the the simple_poll thread
 */
AvahiDiscovery::~AvahiDiscovery() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy AvahiDiscovery object");
}

/**
 * \brief Browse Callback implementation
 */
static void	browse_callback(
			AvahiServiceBrowser *sb,
			AvahiIfIndex interface,
			AvahiProtocol protocol,
			AvahiBrowserEvent event,
			const char *name,
			const char *type,
			const char *domain,
			AvahiLookupResultFlags flags,
			void* userdata) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "browse_callback forwarding");
	AvahiDiscovery	*discovery = ((AvahiDiscovery *)userdata);
	discovery->browse_callback(sb, interface, protocol, event,
		name, type, domain, flags);
}

void	AvahiDiscovery::browse_callback(
			AvahiServiceBrowser *sb,
			AvahiIfIndex interface,
			AvahiProtocol protocol,
			AvahiBrowserEvent event,
			const char *name,
			const char *type,
			const char *domain,
			AvahiLookupResultFlags /* flags */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"browse_callback interface=%d, protocol=%d, name=%s, type=%s, "
		"domain=%s", interface, protocol,
		(name) ? name : "(null)",
		(type) ? type : "(null)",
		(domain) ? domain : "(null)");
	AvahiClient	*client = avahi_service_browser_get_client(sb);
	switch (event) {
	case AVAHI_BROWSER_FAILURE:
		debug(LOG_ERR, DEBUG_LOG, 0, "browser failure: %s",
			avahi_strerror(avahi_client_errno(client)));
		avahi_simple_poll_quit(simple_poll);
		break;
	case AVAHI_BROWSER_NEW:
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"client=%p new service %s of type %s in domain %s",
			client,
			(name) ? name : "(null)",
			(type) ? type : "(null)",
			(domain) ? domain : "(null)");
		{
			ServiceKey	key(name, type, domain);
			key.interface(interface);
			key.protocol(protocol);
			add(key);
		}
		break;
	case AVAHI_BROWSER_REMOVE:
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"remove service %s of type %s in domain %s",
			(name) ? name : "(null)",
			(type) ? type : "(null)",
			(domain) ? domain : "(null)");
		{
			ServiceKey	key(name, type, domain);
			remove(key);
		}
		break;
	case AVAHI_BROWSER_ALL_FOR_NOW:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "all for now");
		break;
	case AVAHI_BROWSER_CACHE_EXHAUSTED:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cache exhausted");
		break;
	}
}

/**
 * \brief Main method of the AvahiDiscovery object
 *
 * This method launches the main loop and also handles cleanup of avahi
 * related objects when the thread exits
 */
void	AvahiDiscovery::main() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main program started for discovery %p",
		this);
	AvahiServiceBrowser	*sb = NULL;
	if (!main_startup()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "main startup failed");
		return;
	}

	// create the service browser
	sb = avahi_service_browser_new(client, AVAHI_IF_UNSPEC,
		AVAHI_PROTO_UNSPEC, "_astro._tcp", NULL,
		(AvahiLookupFlags)0, discover::browse_callback, this);
	if (NULL == sb) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot create browser: %s",
			avahi_strerror(avahi_client_errno(client)));
		goto fail;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "avahi service browser created");

	// event loop for the poll
	debug(LOG_DEBUG, DEBUG_LOG, 0, "running simple_poll loop");
	avahi_simple_poll_loop(simple_poll);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "main program for discover %p complete",
		this);
fail:
	_valid = false;
	if (sb) {
		avahi_service_browser_free(sb);
		sb = NULL;
	}
	if (client) {
		avahi_client_free(client);
		client = NULL;
	}
	if (simple_poll) {
		avahi_simple_poll_free(simple_poll);
		simple_poll = NULL;
	}
}

ServiceObject	AvahiDiscovery::find(const ServiceKey& key) {
	std::unique_lock<std::mutex>	_lock(_mutex);
	auto i = _objects.find(key);
	if (i != _objects.end()) {
		return i->second;
	}
	AvahiResolver	resolver(key, *this);
	resolver.resolve();
	ServiceObject	result = resolver.resolved();
	_objects.insert(std::make_pair(key, result));
	return result;
}

} // namespace discover
} // namespace astro
