/*
 * ServiceSubset.cpp -- class that encapsulates the subset of services
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroDiscovery.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <sstream>

namespace astro {
namespace discover {

ServiceSubset::service_type	ServiceSubset::string2type(
					const std::string& name) {
//	debug(LOG_DEBUG, DEBUG_LOG, 0, "decode: '%s'", name.c_str());
	if (name == "instruments") return INSTRUMENTS;
	if (name == "tasks")       return TASKS;
	if (name == "devices")	   return DEVICES;
	if (name == "guiding")     return GUIDING;
	if (name == "focusing")    return FOCUSING;
	if (name == "images")      return IMAGES;
	if (name == "repository")  return REPOSITORY;
	std::string	msg = stringprintf("invalid service name: %s",
		name.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

std::string	ServiceSubset::type2string(service_type type) {
	switch (type) {
	case INSTRUMENTS:	return std::string("instruments");
	case TASKS:		return std::string("tasks");
	case DEVICES:		return std::string("devices");
	case GUIDING:		return std::string("guiding");
	case FOCUSING:		return std::string("focusing");
	case IMAGES:		return std::string("images");
	case REPOSITORY:	return std::string("repository");
	default:
		std::string	msg = stringprintf("invalid service code: %d",
			(int)type);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
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
//		debug(LOG_DEBUG, DEBUG_LOG, 0, "set: %s", s.c_str());
		set(string2type(s));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set: %s", unsplit(names, ",").c_str());
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
	if (DEVICES     == s) return true;
	if (TASKS       == s) return true;
	if (GUIDING     == s) return true;
	if (FOCUSING    == s) return true;
	if (IMAGES      == s) return true;
	if (REPOSITORY  == s) return true;
	return false;
}


std::list<std::string>	ServiceSubset::types() const {
	std::list<std::string>	result;
	if (has(INSTRUMENTS)) result.push_back(std::string("instruments"));
	if (has(DEVICES)    ) result.push_back(std::string("devices"));
	if (has(TASKS)      ) result.push_back(std::string("tasks"));
	if (has(GUIDING)    ) result.push_back(std::string("guiding"));
	if (has(FOCUSING)   ) result.push_back(std::string("focusing"));
	if (has(IMAGES)     ) result.push_back(std::string("images"));
	if (has(REPOSITORY) ) result.push_back(std::string("repository"));
	return result;
}

void	ServiceSubset::set(service_type s) {
	if (!validtype(s)) {
		std::string	msg = stringprintf("cannot set invalid "
			"service code %d", (int)s);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_services |= s;
}

void	ServiceSubset::unset(service_type s) {
	if (!validtype(s)) {
		std::string	msg = stringprintf("cannot unset invalid "
			"service code %d", (int)s);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_services &= ~s;
}

bool	ServiceSubset::has(service_type s) const {
	if (!validtype(s)) {
		std::string	msg = stringprintf("cannot check for invalid "
			"service code %d", (int)s);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return (_services & s) ? true : false;
}

bool	ServiceSubset::has_any_of(const std::list<service_type>& types) const {
	bool	result = false;
	std::for_each(types.begin(), types.end(),
		[&result,this](const service_type& t) {
			if (this->has(t)) {
				result = true;
			}
		}
	);
	return result;
}

#define	append_to_txt_record(txtbuffer, currentindex, newstring)	\
	txtbuffer[currentindex++] = strlen(newstring);			\
	strcpy(txtbuffer + currentindex, newstring);			\
	currentindex += strlen(newstring);


std::string	ServiceSubset::txtrecord() const {
	char    buffer[100];
	int     l = 0;
	if (has(ServiceSubset::IMAGES)) {
		append_to_txt_record(buffer, l, "images");
	}
	if (has(ServiceSubset::DEVICES)) {
		append_to_txt_record(buffer, l, "devices");
	}
	if (has(ServiceSubset::TASKS)) {
		append_to_txt_record(buffer, l, "tasks");
	}
	if (has(ServiceSubset::INSTRUMENTS)) {
		append_to_txt_record(buffer, l, "instruments");
	}
	if (has(ServiceSubset::GUIDING)) {
		append_to_txt_record(buffer, l, "guiding");
	}
	if (has(ServiceSubset::FOCUSING)) {
		append_to_txt_record(buffer, l, "focusing");
	}
	if (has(ServiceSubset::REPOSITORY)) {
		append_to_txt_record(buffer, l, "repository");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "txt record has length %d", l);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "txt: %s",
		unsplit(result, ", ").c_str());
	return result;
}

} // namespace discover
} // namespace astro
