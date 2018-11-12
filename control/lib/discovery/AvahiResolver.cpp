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
#include <cstring>

namespace astro {
namespace discover {

AvahiResolver::AvahiResolver(const ServiceKey& key, AvahiClient *client)
	: ServiceResolver(key), _client(client) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "AvahiResolver constructed key=%s this=%p",
		key.toString().c_str(), this);
}

AvahiResolver::~AvahiResolver() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "resolver this=%p goes out of scope",
		this);
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
		"userdata(AvahiResolver)=%p",
		(event == AVAHI_RESOLVER_FOUND) ? "FOUND" : "FAILURE",
		(name) ? name : "(null)", (domain) ? domain : "(null)",
		(host_name) ? host_name : "(null)", userdata);
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
		"%p->do_resolve %p, key = %s, interface=%d, protocol=%d",
		this, _client,
		_key.toString().c_str(), _key.interface(), _key.protocol());
	prom = std::shared_ptr<std::promise<bool> >(new std::promise<bool>());
	fut = std::shared_ptr<std::future<bool> >(
		new std::future<bool>(prom->get_future()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "this=%p futures initialized", this);

	char	*name = strdup(_key.name().c_str());
	char	*type = strdup(_key.type().c_str());
	char	*domain = strdup(_key.type().c_str());

	// create a resolver structure
	AvahiServiceResolver	*resolver = avahi_service_resolver_new(_client,
		_key.interface(), _key.protocol(),
		name, type, domain,
		AVAHI_PROTO_UNSPEC,
		(AvahiLookupFlags)0, discover::resolve_callback,
		this);

	if (NULL == resolver) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"this=%p failed to create resolver: %s", this,
			avahi_strerror(avahi_client_errno(_client)));
		throw std::runtime_error("cannot construct a resolver");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%p->resolver created at %p",
		this, resolver);

	// now wait for the resolver to produce a result
	bool	resolved = fut->get();

	// did we resolve?
	if (!resolved) {
		debug(LOG_ERR, DEBUG_LOG, 0, "this=%p failed to resolve", this);
	}
	fut.reset();

	// free the resolver
	avahi_service_resolver_free(resolver);

	// free the names
	free(name);
	free(type);
	free(domain);

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
		"%p->resolve_callback interface=%d protocol=%d, name=%s, "
		"type=%s, domain=%s, host_name=%s",
		this,
		interface, protocol,
		(name) ? name : "(null)",
		(type) ? type : "(null)",
		(domain) ? domain : "(null)",
		(host_name) ? host_name : "(null)");
	if (event == AVAHI_RESOLVER_FAILURE) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "resolver failure");
		prom->set_value(false);
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"service name=%s type=%s resolved this=%p",
		(name) ? name : "(null)", (type) ? type : "(null)", this);
	if (port) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "port: %hu", port);
		_object.port(port);
	}
	if (host_name) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "host name: %s",
			(host_name) ? host_name : "(null)");
		_object.host(host_name);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "this=%p parsing %d txt entries",
		this, avahi_string_list_length(txt));
	while (txt) {
		std::string	s((char *)avahi_string_list_get_text(txt),
					avahi_string_list_get_size(txt));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "this=%p adding txt '%s'",
			this, s.c_str());
		_object.set(s);
		txt = avahi_string_list_get_next(txt);
	}

	// free the resolver, it is not needed any longer
	avahi_service_resolver_free(resolver);

	// resolution complete, signal this to the waiting do_resolve method
	debug(LOG_DEBUG, DEBUG_LOG, 0, "this=%p resolution complete", this);
	prom->set_value(true);
}

} // namespace discover
} // namespace astro
