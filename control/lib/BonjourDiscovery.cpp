/*
 * BonjourDiscovery.cpp -- Bonjour-based service discovery implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <BonjourDiscovery.h>

namespace astro {
namespace discover {

BonjourDiscovery::BonjourDiscovery(service_type t) : ServiceDiscovery(t) {
}

} // namespace discover
} // namespace astro
