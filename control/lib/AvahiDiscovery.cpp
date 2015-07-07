/*
 * AvahiDiscovery.cpp -- Avahi-based service discovery implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AvahiDiscovery.h>
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

static void	avahi_main(AvahiDiscovery *discovery) {
	discovery->main();
}

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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "join the thread");
	if (valid()) {
		avahi_simple_poll_quit(simple_poll);
	}
	// wait for the thread to terminate
	thread.join();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy AvahiDiscovery object");
}

/**
 * \brief Client callback implementation
 */
static void	client_callback(AvahiClient *client, AvahiClientState state,
			void *userdata) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "client_callback called");
	AvahiDiscovery	*discovery = (AvahiDiscovery *)userdata;
	discovery->client_callback(client, state);
}

void	AvahiDiscovery::client_callback(AvahiClient *client,
		AvahiClientState state) {
	assert(client);
	switch (state) {
	case AVAHI_CLIENT_FAILURE:
		debug(LOG_ERR, DEBUG_LOG, 0, "server connection failure: %s",
			avahi_strerror(avahi_client_errno(client)));
		avahi_simple_poll_quit(simple_poll);
		break;

	case AVAHI_CLIENT_S_RUNNING:
	case AVAHI_CLIENT_S_COLLISION:
	case AVAHI_CLIENT_S_REGISTERING:
	case AVAHI_CLIENT_CONNECTING:
		break;
	}
}

/**
 * \brief Resolution callback implementation
 */
static void	resolve_callback(
			AvahiServiceResolver *resolver,
			AvahiIfIndex interface,
			AvahiProtocol protocol,
			AvahiResolverEvent event,
			const char *name,
			const char *type,
			const char *domain,
			const char *host_name,
			const AvahiAddress *address,
			uint16_t port,
			AvahiStringList *txt,
			AvahiLookupResultFlags flags,
			void* userdata) {
	AvahiDiscovery	*discovery = ((AvahiDiscovery *)userdata);
	discovery->resolve_callback(resolver, interface, protocol, event,
		name, type, domain, host_name, address, port, txt, flags);
}

void	AvahiDiscovery::resolve_callback(
			AvahiServiceResolver *resolver,
			AvahiIfIndex interface,
			AvahiProtocol protocol,
			AvahiResolverEvent event,
			const char *name,
			const char *type,
			const char *domain,
			const char *host_name,
			const AvahiAddress *address,
			uint16_t port,
			AvahiStringList *txt,
			AvahiLookupResultFlags flags) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "service %s %s resolved", name, type);
	while (txt) {
		std::string	s((char *)avahi_string_list_get_text(txt),
					avahi_string_list_get_size(txt));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "adding txt '%s'", s.c_str());
		ServiceObject::service_type	t = ServiceObject::type_name(s);
		ServiceObject	o(name, t);
		o.host(host_name);
		add(o);
		txt = avahi_string_list_get_next(txt);
	}
	avahi_service_resolver_free(resolver);
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
			AvahiLookupResultFlags flags) {
	AvahiClient	*client = avahi_service_browser_get_client(sb);
	switch (event) {
	case AVAHI_BROWSER_FAILURE:
		debug(LOG_ERR, DEBUG_LOG, 0, "browser failure: %s",
			avahi_strerror(avahi_client_errno(client)));
		avahi_simple_poll_quit(simple_poll);
		break;
	case AVAHI_BROWSER_NEW:
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"new service %s of type %s in domain %s",
			name, type, domain);

		avahi_service_resolver_new(client, interface, protocol, name,
			type, domain, AVAHI_PROTO_UNSPEC,
			(AvahiLookupFlags)0, discover::resolve_callback,
			this);
		break;
	case AVAHI_BROWSER_REMOVE:
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"remove service %s of type %s in domain %s",
			name, type, domain);
		remove(name);
		break;
	case AVAHI_BROWSER_ALL_FOR_NOW:
		break;
	case AVAHI_BROWSER_CACHE_EXHAUSTED:
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
	_valid = false;

	// create avahi client objects
	AvahiServiceBrowser	*sb = NULL;
	AvahiClient		*client = NULL;
	simple_poll = NULL;

	// create Avahi simple poll object
	simple_poll = avahi_simple_poll_new();
	if (NULL == simple_poll) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"failed to create simple poll object");
		goto fail;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "simple poll created");

	// create avahi client
	int	error;
	client = avahi_client_new(avahi_simple_poll_get(simple_poll),
		(AvahiClientFlags)0, discover::client_callback, this, &error);
	if (NULL == client) {
		debug(LOG_ERR, DEBUG_LOG, 0, "failed to create client: %s",
			avahi_strerror(error));
		goto fail;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "avahi client created");

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
	_valid = true;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "running simple_poll loop");
	avahi_simple_poll_loop(simple_poll);

	_valid = false;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "main program for discover %p complete",
		this);
fail:
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

} // namespace discover
} // namespace astro