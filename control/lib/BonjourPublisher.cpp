/*
 * BonjourPublisher.cpp -- Bonjour-based service discovery implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <BonjourDiscovery.h>
#include <AstroDebug.h>

namespace astro {
namespace discover {

BonjourPublisher::BonjourPublisher(const std::string& servername, int port)
	: ServicePublisher(servername, port) {
	sdRef = NULL;
	_complete = false;
}

BonjourPublisher::~BonjourPublisher() {
	if (sdRef) {
		DNSServiceRefDeallocate(sdRef);
		sdRef = NULL;
	}
}

static void registerreply_callback(DNSServiceRef sdRef,
		DNSServiceFlags flags,
		DNSServiceErrorType errorCode,
		const char *name,
		const char *regtype,
		const char *domain, void *context) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "registerreply callback");
	BonjourPublisher	*publisher = (BonjourPublisher *)context;
	publisher->registerreply_callback(sdRef, flags, errorCode, name,
		regtype, domain);
}

void	BonjourPublisher::registerreply_callback(DNSServiceRef sdRef, 
                        DNSServiceFlags flags, 
                        DNSServiceErrorType errorCode, 
                        const char *name, 
                        const char *regtype, 
                        const char *domain) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "registerreply: %s/%s@%s, flags = %d",
		name, regtype, domain, flags);
	if (!(flags & kDNSServiceFlagsMoreComing)) {
		_complete = true;
	}
}

void	BonjourPublisher::publish() {
	if (sdRef) {
		DNSServiceRefDeallocate(sdRef);
		sdRef = NULL;
	}

	unsigned short	portnumber = port();
	std::string	txt = txtrecord();
	_complete = false;

	DNSServiceErrorType	error = DNSServiceRegister(&sdRef,
		kDNSServiceInterfaceIndexAny, 0,
		servername().c_str(), "_astro._tcp", NULL, NULL,
		htons(portnumber), txt.size(), txt.data(),
		discover::registerreply_callback, this);

	if (error != kDNSServiceErr_NoError) {
		debug(LOG_ERR, DEBUG_LOG, 0, "registring failed: %d", error);
		DNSServiceRefDeallocate(sdRef);
		sdRef = NULL;
		return;
	}

	do {
		error = DNSServiceProcessResult(sdRef);
	} while (!_complete);
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "registration complete");
	return;
}

} // namespace discover
} // namespace astro
