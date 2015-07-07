/*
 * BonjourPublisher.cpp -- Bonjour-based service discovery implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <BonjourDiscovery.h>

namespace astro {
namespace discover {

BonjourPublisher::BonjourPublisher(const std::sring& servername, int port)
	: ServicePublisher(servername, port) {
}

} // namespace discover
} // namespace astro