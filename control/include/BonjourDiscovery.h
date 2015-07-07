/*
 * BonjourDiscovery.h -- Bonjour-based service discovery implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _BonjourDiscovery_h
#define _BonjourDiscovery_h

#include <ServiceDiscovery.h>
#include <dns_sd.h>

namespace astro {
namespace discover {

class BonjourDiscovery : public ServiceDiscovery {
public:
	BonjourDiscovery();
};

class BonjourPublisher : public ServicePublisher {
	DNSServiceRef	sdRef;
public:
	BonjourPublisher(const std::string& servername, int port);
	~BonjourPublisher();
	virtual void	publish();
	void	registerreply_callback(DNSServiceRef sdRef,
			DNSServiceFlags flags,
			DNSServiceErrorType errorCode,
			const char *name,
			const char *regtype,
			const char *domain);
};

} // namespace discover
} // namespace astro

#endif /* _BonjourDiscovery_h */
