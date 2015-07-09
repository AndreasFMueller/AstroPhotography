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
 * \brief Trampoline-Callback for browse replies
 */
static void	browsereply_callback(DNSServiceRef sdRef,
			DNSServiceFlags flags,
			uint32_t interfaceIndex,
			DNSServiceErrorType errorCode,
			const char *serviceName,
			const char *regtype,
			const char *replyDomain,
			void *context) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "browsereply_callback called");
	BonjourDiscovery	*discovery = (BonjourDiscovery *)context;
	discovery->browsereply_callback(sdRef, flags, interfaceIndex,
		errorCode, serviceName, regtype, replyDomain);
}

/**
 * \brief Callback for browse replies
 *
 * This callback is called when then browser detects a change in the 
 * set of service providers published on the net.
 */
void    BonjourDiscovery::browsereply_callback(DNSServiceRef sdRef,
			DNSServiceFlags flags,
			uint32_t interfaceIndex,
			DNSServiceErrorType errorCode,
			const char *serviceName,
			const char *regtype,
			const char *replyDomain) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d found service %s/%s@%s", flags,
		serviceName, regtype, replyDomain);
	
	if (flags && kDNSServiceFlagsAdd) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "add service");
		add(ServiceKey(serviceName, regtype, replyDomain));
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "remove service");
		remove(ServiceKey(serviceName, regtype, replyDomain));
	}
}

/**
 * \brief Main method for the 
 */
void	BonjourDiscovery::main() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start thread");
	assert(sdRef != NULL);
	int	error;
	do {
		error = DNSServiceProcessResult(sdRef);
	} while (error == kDNSServiceErr_NoError);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end thread");
}

/**
 * \brief trampoline main function 
 */
static void	main(BonjourDiscovery *discovery) {
	discovery->main();
}

ServiceObject	BonjourDiscovery::find(const ServiceKey& key) {
	BonjourResolver	resolver(key);
	return resolver.resolved();
}

BonjourDiscovery::BonjourDiscovery() : ServiceDiscovery() {
	thread = NULL;
	sdRef = NULL;
	DNSServiceErrorType	error = DNSServiceBrowse(&sdRef, 0,
		kDNSServiceInterfaceIndexAny,
		"_astro._tcp", NULL, discover::browsereply_callback, this);
	if (error != kDNSServiceErr_NoError) {
		debug(LOG_ERR, DEBUG_LOG, 0, "browser failed: %d", error);
		throw std::runtime_error("cannot create browser");
	}
	assert(sdRef != NULL);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "DNSServiceBrowse started");

	// start a thread
	thread = new std::thread(discover::main, this);
}

BonjourDiscovery::~BonjourDiscovery() {
	if (sdRef) {
		close(DNSServiceRefSockFD(sdRef));
	}
	if (thread) {
		thread->join();
		delete thread;
		thread = NULL;
	}
	if (sdRef) {
		DNSServiceRefDeallocate(sdRef);
		sdRef = NULL;
	}
}

} // namespace discover
} // namespace astro
