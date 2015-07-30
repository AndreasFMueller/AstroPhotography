/*
 * ServiceObject.cpp -- class encapsulating the service description
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <AstroDiscovery.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroUtils.h>
#include <stdexcept>
#include <algorithm>

namespace astro {
namespace discover {

/**
 * \brief Create a SerivceObject from name and port
 */
ServiceObject::ServiceObject(const ServiceKey& key) : ServiceKey(key) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new service object from key %s",
		key.toString().c_str());
	_port = 0;
}

/**
 * \brief Convert the service object into a string representation
 */
std::string	ServiceObject::toString() const {
	return stringprintf("%s @ %s:%d", ServiceKey::toString().c_str(),
		host().c_str(), port());
}

/**
 * \brief Comparision operator for Service objects
 */
bool	ServiceObject::operator<(const ServiceObject& other) const {
	return ServiceKey::operator<(other);
}

std::string	ServiceObject::connect(const std::string& service) const {
	return astro::ServerName(_host, _port).connect(service);
}

} // namespace discover
} // namespace astro
