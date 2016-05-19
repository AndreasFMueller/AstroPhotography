/*
 * AvahiServiceSubset.cpp -- convert to an avahi string list
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "AvahiDiscovery.h"
#include <list>

namespace astro {
namespace discover {

AvahiServiceSubset::AvahiServiceSubset(const std::list<std::string>& names)
	: ServiceSubset(names) {
}

AvahiStringList	*AvahiServiceSubset::stringlist(const ServiceSubset& s) {
	AvahiStringList	*strlist = NULL;
	if (s.has(ServiceSubset::INSTRUMENTS)) {
		strlist = avahi_string_list_add(strlist, "instruments");
	}
	if (s.has(ServiceSubset::TASKS)) {
		strlist = avahi_string_list_add(strlist, "tasks");
	}
	if (s.has(ServiceSubset::GUIDING)) {
		strlist = avahi_string_list_add(strlist, "guiding");
	}
	if (s.has(ServiceSubset::IMAGES)) {
		strlist = avahi_string_list_add(strlist, "images");
	}
	if (s.has(ServiceSubset::DEVICES)) {
		strlist = avahi_string_list_add(strlist, "devices");
	}
	if (s.has(ServiceSubset::FOCUSING)) {
		strlist = avahi_string_list_add(strlist, "focusing");
	}
	if (s.has(ServiceSubset::REPOSITORY)) {
		strlist = avahi_string_list_add(strlist, "repository");
	}
	return strlist;
}

AvahiStringList	*AvahiServiceSubset::stringlist() const {
	return stringlist(*this);
}

} // namespace discover
} // namespace astro
