/*
 * ServiceKey.cpp -- service key implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller
 */
#include <AstroDiscovery.h>

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

bool	ServiceKey::operator==(const ServiceKey& other) const {
	if (_name != other._name) { return false; }
	if (_type != other._type) { return false; }
	if (_domain != other._domain) { return false; }
	return true;
}

std::string	ServiceKey::toString() const {
	return _name + "/" + _type + "@" + _domain;
}

#if 0
ServiceKey::ServiceKey(const ServiceKey& other) {
	_name = other._name;
	_type = other._type;
	_domain = other._domain;
	_interface = other._interface;
	_protocol = other._protocol;
}

ServiceKey&	ServiceKey::operator=(const ServiceKey& other) {
	_name = other._name;
	_type = other._type;
	_domain = other._domain;
	_interface = other._interface;
	_protocol = other._protocol;
	return *this;
}
#endif

} // namespace discover
} // namespace astro
