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


void    BonjourDiscovery::browsereply_callback(DNSServiceRef sdRef,
			DNSServiceFlags flags,
			uint32_t interfaceIndex,
			DNSServiceErrorType errorCode,
			const char *serviceName,
			const char *regtype,
			const char *replyDomain) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found servive %s/%s@%s", serviceName,
		regtype, replyDomain);
}

void	BonjourDiscovery::main() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start thread");
	int	error;
	do {
		error = DNSServiceProcessResult(sdRef);
	} while (error == kDNSServiceErr_NoError);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end thread");
}

static void	bonjour_main(BonjourDiscovery *discovery) {
	discovery->main();
}

BonjourDiscovery::BonjourDiscovery() : ServiceDiscovery() {
	sdRef = NULL;
	DNSServiceErrorType	error = DNSServiceBrowse(&sdRef, 0, 0,
		"_astro._tcp", NULL, discover::browsereply_callback, this);
	if (error != kDNSServiceErr_NoError) {
		debug(LOG_ERR, DEBUG_LOG, 0, "browser failed: %d", error);
		throw std::runtime_error("cannot create browser");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "DNSServiceBrowse started");

	// start a thread
	thread = new std::thread(bonjour_main, this);
	return;
}

BonjourDiscovery::~BonjourDiscovery() {
	close(DNSServiceRefSockFD(sdRef));
	thread->join();
	delete thread;
	if (sdRef) {
		DNSServiceRefDeallocate(sdRef);
		sdRef = NULL;
	}
}

} // namespace discover
} // namespace astro
