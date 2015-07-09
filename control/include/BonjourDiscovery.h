/*
 * BonjourDiscovery.h -- Bonjour-based service discovery implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _BonjourDiscovery_h
#define _BonjourDiscovery_h

#include <ServiceDiscovery.h>
#include <dns_sd.h>
#include <future>

namespace astro {
namespace discover {

/**
 * \brief Resolver class for Bonjour implementation
 *
 * This class uses a std::future object to synchronize with the resolving
 * thread
 */
class BonjourResolver : public ServiceResolver {
	DNSServiceRef   sdRef;
public:
	BonjourResolver(const ServiceKey& key);
	~BonjourResolver();
	void    resolvereply_callback(
			DNSServiceRef sdRef,
			DNSServiceFlags flags,
			uint32_t interfaceIndex,
			DNSServiceErrorType errorCode,
			const char *fullname,
			const char *hosttarget,
			uint16_t port,
			uint16_t txtLen,
			const unsigned char *txtRecord
		);
	virtual ServiceObject    do_resolve();
};


/**
 * \brief Implementation class for Bonjour based service discovery
 *
 * Discovery using Bonjour is the method of choice on the Mac. Although
 * the same API is available for Linux as well, Avahi is much more common
 * on that plattform. But since Avahi is not reasonably available on 
 * Mac OS X, wie have to write a separate implementation.
 */
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
	virtual ServiceObject	find(const ServiceKey& key);
};

/**
 * \brief Implementation class for Bonjour based service publishing
 */
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
