/*
 * IceDiscovery.h -- auxiliary class to discover a server
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDiscovery.h>

namespace snowstar {

class IceDiscovery {
public:
static	astro::discover::ServiceObject	discover(const std::string& name);
};

} // namespace snowstar
