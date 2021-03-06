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
	_interface = -1;
	_protocol = -1;
}

ServiceKey::ServiceKey(const std::string& nametypedomain) {
	size_t	slashoffset = nametypedomain.find('/');
	size_t	atoffset = nametypedomain.find('@');
	_name = nametypedomain.substr(0, slashoffset);
	_type = nametypedomain.substr(slashoffset + 1,
		atoffset - slashoffset - 1);
	_domain = nametypedomain.substr(atoffset + 1);
	_interface = -1;
	_protocol = -1;
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

} // namespace discover
} // namespace astro
