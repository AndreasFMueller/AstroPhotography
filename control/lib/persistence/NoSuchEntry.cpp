/*
 * NoSuchEntry.cpp -- implementation of the NoSuchEntry exception
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConfig.h>
#include <AstroPersistence.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <cstdlib>

namespace astro {
namespace config {

static std::string nosuchmessage_format(const std::string& domain,
	const std::string& section, const std::string& name) {
	return stringprintf("no entry %s/%s/%s",
		domain.c_str(), section.c_str(), name.c_str());
}

NoSuchEntry::NoSuchEntry(const std::string& domain, const std::string& section,
	const std::string& name)
	: std::runtime_error(nosuchmessage_format(domain, section, name)) {
}

NoSuchEntry::NoSuchEntry(const std::string& msg) : std::runtime_error(msg) {
}

NoSuchEntry::NoSuchEntry() : std::runtime_error("no such config entry") {
}

} // namespace config
} // namespace astro
