/*
 * AvahiResolver.cpp -- resolver implementation for avahi
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AvahiDiscovery.h>
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
}

AvahiResolver::~AvahiResolver() {
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
	AvahiResolver	*ares = ((AvahiResolver *)userdata);
	ares->resolve_callback(resolver, interface, protocol, event,
		name, type, domain, host_name, address, port, txt, flags);
}

ServiceObject	AvahiResolver::do_resolve() {
	avahi_service_resolver_new(_client, 0, 0,
		_key.name().c_str(), _key.type().c_str(), _key.domain().c_str(),
		AVAHI_PROTO_UNSPEC,
		(AvahiLookupFlags)0, discover::resolve_callback,
		this);
	return _object;
}

void	AvahiResolver::resolve_callback(
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
	_object.port(port);
	_object.host(host_name);
	while (txt) {
		std::string	s((char *)avahi_string_list_get_text(txt),
					avahi_string_list_get_size(txt));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "adding txt '%s'", s.c_str());
		_object.set(s);
		txt = avahi_string_list_get_next(txt);
	}
	avahi_service_resolver_free(resolver);
}


} // namespace discover
} // namespace astro
