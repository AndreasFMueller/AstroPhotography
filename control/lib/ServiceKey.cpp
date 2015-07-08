/*
 * ServiceKey.cpp -- service key implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller
 */
#include <ServiceDiscovery.h>

namespace astro {
namespace discover {

ServiceKey::ServiceKey(const std::string& name, const std::string& type,
	const std::string& domain)
	: _name(name), _type(type), _domain(domain) {
}

bool	ServiceKey::operator<(const ServiceKey& other) const {
	if (_domain < other._domain) {
		return true;
	}
	if (_domain > other._domain) {
		return false;
	}
	if (_name < other._name) {
		return true;
	}
	if (_name > other._name) {
		return false;
	}
	return (_type < other._type);
}

std::string	ServiceKey::toString() const {
	return _name + "/" + _type + "@" + _domain;
}

} // namespace discover
} // namespace astro
