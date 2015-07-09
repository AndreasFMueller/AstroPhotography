/*
 * BonjourDiscovery.cpp -- Bonjour-based service discovery implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDebug.h>
#include <BonjourDiscovery.h>
#include <dns_sd.h>
#include <includes.h>
#include <sys/select.h>

namespace astro {
namespace discover {

/**
 * \brief trampoline function for ResolveReply callbacks
 */
static void	resolvereply_callback(
			DNSServiceRef sdRef,
			DNSServiceFlags flags,
			uint32_t interfaceIndex,
			DNSServiceErrorType errorCode,
			const char *fullname,
			const char *hosttarget,
			uint16_t port,
			uint16_t txtLen,
			const unsigned char *txtRecord,
			void *context) {
	BonjourResolver	*resolver = (BonjourResolver *)context;
	resolver->resolvereply_callback(sdRef, flags, interfaceIndex,
		errorCode, fullname, hosttarget, port, txtLen, txtRecord);
}

/**
 * \brief ResolveReply callback
 */
void	BonjourResolver::resolvereply_callback(
		DNSServiceRef sdRef,
		DNSServiceFlags flags,
		uint32_t interfaceIndex,
		DNSServiceErrorType errorCode,
		const char *fullname,
		const char *hosttarget,
		uint16_t port,
		uint16_t txtLen,
		const unsigned char *txtRecord) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "resolvereply: %d", flags);
	if (!(flags & kDNSServiceFlagsAdd)) {
		return;
	}
	if (port) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "have port: %d", ntohs(port));
		_object.port(ntohs(port));
	}
	if (hosttarget) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "have host: %s", hosttarget);
		_object.host(hosttarget);
	}
	int	i = 0;
	while (i < txtLen) {
		int	l = txtRecord[i];
		if (l > 0) {
			std::string	name((char *)txtRecord + i + 1, l);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "txt[%d](%d) = '%s'",
				i, l, name.c_str());
			_object.set(name);
			i += l + 1;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "object: %s",
		_object.toString().c_str());
	if (!(flags & kDNSServiceFlagsMoreComing)) {
		DNSServiceRefDeallocate(sdRef);
		sdRef = NULL;
	}
}


/**
 * \brief main resolve function
 */
ServiceObject	BonjourResolver::do_resolve() {
	sdRef = NULL;
	DNSServiceResolve(&sdRef, 0, 0, _key.name().c_str(),
		_key.type().c_str(), _key.domain().c_str(),
		discover::resolvereply_callback, this);
	int	error;
	do {
		error = DNSServiceProcessResult(sdRef);
	} while (error == kDNSServiceErr_NoError);
	return _object;
}

/**
 * \brief Construct a resolver object
 */
BonjourResolver::BonjourResolver(const ServiceKey& key) : ServiceResolver(key) {
}

/**
 * \brief Destroy the resolver object
 */
BonjourResolver::~BonjourResolver() {
	if (sdRef) {
		close(DNSServiceRefSockFD(sdRef));
		DNSServiceRefDeallocate(sdRef);
		sdRef = NULL;
	}
}

} // namespace discover
} // namespace astro
