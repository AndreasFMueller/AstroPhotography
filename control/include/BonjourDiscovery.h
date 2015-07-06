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
	BonjourDiscovery(service_type t);
};

} // namespace discover
} // namespace astro

#endif /* _BonjourDiscovery_h */
