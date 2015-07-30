/*
 * ServiceSubset.cpp -- class that encapsulates the subset of services
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroDiscovery.h>
#include <AstroDebug.h>
#include <sstream>

namespace astro {
namespace discover {

ServiceSubset::service_type	ServiceSubset::string2type(
					const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "decode: '%s'", name.c_str());
	if (name == "instruments") return INSTRUMENTS;
	if (name == "tasks")       return TASKS;
	if (name == "guiding")     return GUIDING;
	if (name == "images")      return IMAGES;
	throw std::runtime_error("invalid string type name");
}

std::string	ServiceSubset::type2string(service_type type) {
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

void	ServiceSubset::set(const std::list<std::string>& names) {
	std::list<std::string>::const_iterator	i;
	for (i = names.begin(); i != names.end(); i++) {
		std::string	s = *i;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set: %s", s.c_str());
		set(string2type(s));
	}
}

void	ServiceSubset::unset(const std::list<std::string>& names) {
	std::list<std::string>::const_iterator	i;
	for (i = names.begin(); i != names.end(); i++) {
		unset(string2type(*i));
	}
}

ServiceSubset::ServiceSubset() {
	_services = 0;
}

ServiceSubset::ServiceSubset(const std::list<std::string>& names) {
	_services = 0;
	set(names);
}

ServiceSubset::ServiceSubset(const std::string& txt) {
	_services = 0;
	set(txtparse(txt));
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

std::string	ServiceSubset::txtrecord() const {
	char    buffer[100];
	int     l = 0;
	if (has(ServiceSubset::IMAGES)) {
		buffer[l] = 6;
		strcpy(buffer + l + 1, "images");
		l += 7;
	}
	if (has(ServiceSubset::TASKS)) {
		buffer[l] = 5;
		strcpy(buffer + l + 1, "tasks");
		l += 6;
	}
	if (has(ServiceSubset::INSTRUMENTS)) {
		buffer[l] = 11;
		strcpy(buffer + l + 1, "instruments");
		l += 12;
	}
	if (has(ServiceSubset::GUIDING)) {
		buffer[l] = 7;
		strcpy(buffer + l + 1, "guiding");
		l += 8;
	}
	return std::string(buffer, l);
}

std::list<std::string>	ServiceSubset::txtparse(const std::string& txt) {
	std::list<std::string>	result;
	const char	*txtRecord = txt.data();
	size_t	txtLen = txt.size();
	size_t	i = 0;
	while (i < txtLen) {
		int     l = txtRecord[i];
		if (l > 0) {
			std::string     name((char *)txtRecord + i + 1, l);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "txt[%d](%d) = '%s'",
				i, l, name.c_str());
			result.push_back(name);
		}
		i += l + 1;
	}
	return result;
}

} // namespace discover
} // namespace astro
