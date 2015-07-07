/*
 * BonjourDiscovery.h -- Bonjour-based service discovery implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _BonjourDiscovery_h
#define _BonjourDiscovery_h

#include <ServiceDiscovery.h>

namespace astro {
namespace discover {

class BonjourDiscovery : public ServiceDiscovery {
public:
	BonjourDiscovery();
};

class BonjourPublisher : public ServicePublisher {
public:
	BonjourPublisher(const std::string& servername, int port);
};

} // namespace discover
} // namespace astro

#endif /* _BonjourDiscovery_h */
