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
}

BonjourPublisher::~BonjourPublisher() {
	if (sdRef) {
		DNSServiceRefDeallocate(sdRef);
	}
}

static void registerreply_callback(DNSServiceRef sdRef,
		DNSServiceFlags flags,
		DNSServiceErrorType errorCode,
		const char *name,
		const char *regtype,
		const char *domain, void *context) {
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "service found: %s/%s@%s",
		name, regtype, domain);
}

void	BonjourPublisher::publish() {
	if (sdRef) {
		DNSServiceRefDeallocate(sdRef);
	}
	sdRef = NULL;
	char	buffer[100];
	int	l = 0;
	if (has(ServiceSubset::IMAGES)) {
		buffer[l] = 6;
		strcpy(buffer + l + 1, "images");
		l += 7;
	}
	if (has(ServiceSubset::TASKS)) {
		buffer[l] = 5;
		strcpy(buffer + l + 1, "tasks");
		l += 6;
	}
	if (has(ServiceSubset::INSTRUMENTS)) {
		buffer[l] = 11;
		strcpy(buffer + l + 1, "instruments");
		l += 12;
	}
	if (has(ServiceSubset::GUIDING)) {
		buffer[l] = 7;
		strcpy(buffer + l + 1, "guiding");
		l += 8;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "length = %d", l);
	unsigned short	portnumber = port();
	DNSServiceRegister(&sdRef, 0, 0, servername().c_str(), "_astro._tcp",
		NULL, NULL, htons(portnumber), l, buffer,
		discover::registerreply_callback, this);
}

} // namespace discover
} // namespace astro
