/*
 * BonjourDiscovery.h -- Bonjour-based service discovery implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _BonjourDiscovery_h
#define _BonjourDiscovery_h

#include <ServiceDiscovery.h>
#include <dns_sd.h>
#include <thread>

namespace astro {
namespace discover {

class BonjourDiscovery : public ServiceDiscovery {
	DNSServiceRef	sdRef;
	std::thread	*thread;
public:
	BonjourDiscovery();
	~BonjourDiscovery();
	void	browsereply_callback(DNSServiceRef sdRef, 
			DNSServiceFlags flags, 
			uint32_t interfaceIndex, 
			DNSServiceErrorType errorCode, 
			const char *serviceName, 
			const char *regtype, 
			const char *replyDomain);
	void	main();
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
