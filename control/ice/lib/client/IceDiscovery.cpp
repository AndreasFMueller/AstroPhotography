/*
 * IceDiscovery.cpp -- auxiliary class to discover a server
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <IceDiscovery.h>
#include <AstroDebug.h>

namespace snowstar {

astro::discover::ServiceObject	IceDiscovery::discover(
						const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "service: %s", name.c_str());
	astro::discover::ServiceDiscoveryPtr	discovery
		= astro::discover::ServiceDiscovery::get();
	astro::discover::ServiceKey	key = discovery->waitfor(name);
	astro::discover::ServiceObject	serviceobject = discovery->find(key);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "host: %s, port: %d",
		serviceobject.host().c_str(), serviceobject.port());
	return serviceobject;
}

} // namespace snowstar
