/*
 * AvahiResolver.cpp -- resolver implementation for avahi
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "AvahiDiscovery.h"
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-client/publish.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>
#include <AstroDebug.h>

namespace astro {
namespace discover {

AvahiResolver::AvahiResolver(const ServiceKey& key, AvahiClient *client)
	: ServiceResolver(key), _client(client) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "AvahiResolver constructed for %s at %p",
		key.toString().c_str(), this);
}

AvahiResolver::~AvahiResolver() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "resolver %p goes out of scope", this);
}

/**
 * \brief Resolution callback implementation
 *
 * This method uses the userdata field to forward the callback to the
 * class specific callback method.
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
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"resolver callback event=%s, name=%s, domain=%s, host_name=%s, "
		"userdata=%p",
		(event == AVAHI_RESOLVER_FOUND) ? "FOUND" : "FAILURE",
		name, domain, host_name, userdata);
	if (NULL == userdata) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"no resolver provided in userdata, giving up");
		return;
	}
	AvahiResolver	*ares = ((AvahiResolver *)userdata);
	ares->resolve_callback(resolver, interface, protocol, event,
		name, type, domain, host_name, address, port, txt, flags);
}

/**
 * \brief do_resolve method
 *
 * This method essentially sets up the synchronization mechanism between the
 * callback and this method, creates a resolver structure and waits for the
 * synchronization mechanism to be used.
 */
ServiceObject	AvahiResolver::do_resolve() {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"start do_resolve %p, key = %s, interface=%d, protocol=%d",
		_client, _key.toString().c_str(), _key.interface(), _key.protocol());
	prom = std::shared_ptr<std::promise<bool> >(new std::promise<bool>());
	fut = std::shared_ptr<std::future<bool> >(
		new std::future<bool>(prom->get_future()));

	// create a resolver structure
	AvahiServiceResolver	*resolver = avahi_service_resolver_new(_client,
		_key.interface(), _key.protocol(),
		_key.name().c_str(), _key.type().c_str(), _key.domain().c_str(),
		AVAHI_PROTO_UNSPEC,
		(AvahiLookupFlags)0, discover::resolve_callback,
		this);

	if (NULL == resolver) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "failed to create resolver: %s",
			avahi_strerror(avahi_client_errno(_client)));
		throw std::runtime_error("cannot construct a resolver");
	}

	// now wat for the resolver to produce a result
	fut->get();
	fut.reset();

	// done, return the info
	return _object;
}

/**
 * \brief class specific resolve callback
 *
 * When this callback determines that the name has been resolved, it sets
 * the future variable to true and thus releases the waiting do_resolve
 * method.
 */
void	AvahiResolver::resolve_callback(
			AvahiServiceResolver *resolver,
			AvahiIfIndex interface,
			AvahiProtocol protocol,
			AvahiResolverEvent event,
			const char *name,
			const char *type,
			const char *domain,
			const char *host_name,
			const AvahiAddress * /* address */,
			uint16_t port,
			AvahiStringList *txt,
			AvahiLookupResultFlags /* flags */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"resolve_callback interface=%d protocol=%d, name=%s, type=%s, "
		"domain=%s, host_name=%s",
		interface, protocol, name, type, domain, host_name);
	if (event == AVAHI_RESOLVER_FAILURE) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "resolver failure");
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "service %s %s resolved", name, type);
	if (port) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "port: %hu", port);
		_object.port(port);
	}
	if (host_name) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "host name: %s", host_name);
		_object.host(host_name);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parsing %d txt entries",
		avahi_string_list_length(txt));
	while (txt) {
		std::string	s((char *)avahi_string_list_get_text(txt),
					avahi_string_list_get_size(txt));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "adding txt '%s'", s.c_str());
		_object.set(s);
		txt = avahi_string_list_get_next(txt);
	}

	// free the resolver, it is not needed any longer
	avahi_service_resolver_free(resolver);

	// resolution complete, signal this to the waiting do_resolve method
	debug(LOG_DEBUG, DEBUG_LOG, 0, "resolution complete");
	prom->set_value(true);
}


} // namespace discover
} // namespace astro
