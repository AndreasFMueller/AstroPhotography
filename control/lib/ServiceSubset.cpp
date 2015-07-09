/*
 * ServiceSubset.cpp -- class that encapsulates the subset of services
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ServiceDiscovery.h>
#include <AstroDebug.h>
#include <sstream>

namespace astro {
namespace discover {

ServiceSubset::service_type	ServiceSubset::string2type(
					const std::string& name) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "decode: '%s'", name.c_str());
	if (name == "instruments") return INSTRUMENTS;
	if (name == "tasks")       return TASKS;
	if (name == "guiding")     return GUIDING;
	if (name == "images")      return IMAGES;
	throw std::runtime_error("invalid string type name");
}

std::string	ServiceSubset::type2string(service_type type) const {
	switch (type) {
	case INSTRUMENTS:	return std::string("instruments");
	case TASKS:		return std::string("tasks");
	case GUIDING:		return std::string("guiding");
	case IMAGES:		return std::string("images");
	default:	throw std::runtime_error("invalid type code");
	}
}

std::string	ServiceSubset::toString() const {
	std::ostringstream	out;
	std::list<std::string>	t = types();
	out << "[";
	std::list<std::string>::const_iterator	i;
	for (i = t.begin(); i != t.end(); i++) {
		if (i != t.begin()) {
			out << " ,";
		}
		out << "'" << *i << "'";
	}
	out << "]";
	return out.str();
}

ServiceSubset::ServiceSubset() {
	_services = 0;
}

ServiceSubset::ServiceSubset(const std::list<std::string>& names) {
	std::list<std::string>::const_iterator	i;
	for (i = names.begin(); i != names.end(); i++) {
		set(string2type(*i));
	}
}

bool	ServiceSubset::validtype(service_type s) const {
	if (INSTRUMENTS == s) return true;
	if (TASKS       == s) return true;
	if (GUIDING     == s) return true;
	if (IMAGES      == s) return true;
	return false;
}


std::list<std::string>	ServiceSubset::types() const {
	std::list<std::string>	result;
	if (has(INSTRUMENTS)) result.push_back(std::string("instruments"));
	if (has(TASKS)      ) result.push_back(std::string("tasks"));
	if (has(GUIDING)    ) result.push_back(std::string("guiding"));
	if (has(IMAGES)     ) result.push_back(std::string("images"));
	return result;
}

void	ServiceSubset::set(service_type s) {
	if (!validtype(s)) {
		throw std::runtime_error("invalid service code");
	}
	_services |= s;
}

void	ServiceSubset::unset(service_type s) {
	if (!validtype(s)) {
		throw std::runtime_error("invalid service code");
	}
	_services &= ~s;
}

bool	ServiceSubset::has(service_type s) const {
	if (!validtype(s)) {
		throw std::runtime_error("invalid service code");
	}
	return (_services & s) ? true : false;
}

} // namespace discover
} // namespace astro
