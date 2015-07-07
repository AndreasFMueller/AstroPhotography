/*
 * ServiceObject.cpp -- class encapsulating the service description
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <ServiceDiscovery.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <stdexcept>
#include <algorithm>

namespace astro {
namespace discover {

/**
 * \brief Create a SerivceObject from name and port
 */
ServiceObject::ServiceObject(const std::string& name,
	ServiceObject::service_type type) 
	: _type(type), _name(name) {
	_port = 0;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new service %s", toString().c_str());
}

ServiceObject::ServiceObject(const std::string& name, const std::string& tn)
	: _type(ServiceObject::type_name(tn)), _name(name) {
	_port = 0;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new service %s", toString().c_str());
}

/**
 * \brief Convert the service object into a string representation
 */
std::string	ServiceObject::toString() const {
	return stringprintf("%s/%s:%d@%s", name().c_str(),
		type_name(_type).c_str(), port(), host().c_str());
}

/**
 * \brief Comparision operator for Service objects
 */
bool	ServiceObject::operator<(const ServiceObject& other) const {
	if (_type < other._type) {
		return true;
	}
	if (_type > other._type) {
		return false;
	}
	return (_name < other._name);
}

/**
 * \brief Convert the service type into a descriptive string
 */
std::string	ServiceObject::type_name(service_type t) {
	switch (t) {
	case INSTRUMENTS:
		return std::string("instruments");
	case TASKS:
		return std::string("tasks");
	case GUIDING:
		return std::string("guiding");
	case IMAGES:
		return std::string("images");
	}
	throw std::runtime_error("bad type code");
}

/**
 * \brief Convert a type name into a type code
 */
ServiceObject::service_type	ServiceObject::type_name(const std::string& n) {
	if (n == "instruments") {
		return INSTRUMENTS;
	}
	if (n == "tasks") {
		return TASKS;
	}
	if (n == "guiding") {
		return GUIDING;
	}
	if (n == "images") {
		return IMAGES;
	}
	throw std::range_error("illegal type name");
}

} // namespace discover
} // namespace astro
