/*
 * BonjourDiscovery.cpp -- Bonjour-based service discovery implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDebug.h>
#include "BonjourDiscovery.h"
#include <dns_sd.h>
#include <includes.h>
#include <sys/select.h>

namespace astro {
namespace discover {

/**
 * \brief trampoline function for ResolveReply callbacks
 *
 * \param sdRef
 * \param flags
 * \param interfaceIndex
 * \param errorCode
 * \param fullname
 * \param hosttarget
 * \param port
 * \param txtLen
 * \param TxtRecord
 * \param context
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
 *
 * \param sdRef
 * \param flags
 * \param interfaceIndex
 * \param errorCode
 * \param fullname
 * \param hosttarget
 * \param port
 * \param txtLen
 * \param txtRecord
 */
void	BonjourResolver::resolvereply_callback(
		DNSServiceRef sdRef,
		DNSServiceFlags flags,
		uint32_t /* interfaceIndex */,
		DNSServiceErrorType /* errorCode */,
		const char * /* fullname */,
		const char *hosttarget,
		uint16_t port,
		uint16_t txtLen,
		const unsigned char *txtRecord) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "resolvereply: %d", flags);
	// XXX there seems to be an error here
	if (!(flags & kDNSServiceFlagsAdd)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"XXX FlagsAdd not set, skipping XXX");
		// XXX
		//return;
	}
	if (port) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "have port: %d", ntohs(port));
		_object.port(ntohs(port));
	}
	if (hosttarget) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "have host: %s", hosttarget);
		_object.host(hosttarget);
	}
	std::string	txt((char *)txtRecord, txtLen);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "txt set, length %d (%d)",
		txt.size(), txtLen);
	_object.set(ServiceSubset::txtparse(txt));

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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start resolving");
	DNSServiceRef	sdRef = NULL;
	DNSServiceResolve(&sdRef, 0, kDNSServiceInterfaceIndexAny,
		_key.name().c_str(), _key.type().c_str(),
		_key.domain().c_str(),
		discover::resolvereply_callback, this);
	int	error;
	do {
		error = DNSServiceProcessResult(sdRef);
	} while (error == kDNSServiceErr_NoError);
	if (sdRef) {
		close(DNSServiceRefSockFD(sdRef));
		DNSServiceRefDeallocate(sdRef);
		sdRef = NULL;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "resolution complete");
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
}

} // namespace discover
} // namespace astro
