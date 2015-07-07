/*
 * BonjourDiscovery.cpp -- Bonjour-based service discovery implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <BonjourDiscovery.h>
#include <dns_sd.h>

namespace astro {
namespace discover {

BonjourDiscovery::BonjourDiscovery() : ServiceDiscovery() {
	// create a thread to discover services
}



} // namespace discover
} // namespace astro
