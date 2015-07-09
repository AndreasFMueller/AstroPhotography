/*
 * ServiceResolver.cpp -- resolution base class
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ServiceDiscovery.h>

namespace astro {
namespace discover {

/**
 * \brief Resolve function used for the future object
 */
ServiceObject   do_resolve(ServiceResolver *resolver) {
        return resolver->do_resolve();
}

ServiceResolver::ServiceResolver(const ServiceKey& key)
	: _key(key), _object(key) {
	_resolved = std::async(discover::do_resolve, this);
}

ServiceResolver::~ServiceResolver() {
}

ServiceObject	ServiceResolver::resolved() {
	return _resolved.get();
}

} // namespace discover
} // namespace astro
