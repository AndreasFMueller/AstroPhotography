/*
 * ConfigConversions.cpp -- conversions between ice and astro
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <IceConversions.h>
#include <includes.h>
#include <AstroIO.h>
#include <AstroUtils.h>

namespace snowstar {

struct ConfigurationKey	convert(const astro::config::ConfigurationKey& key) {
	struct ConfigurationKey	result;
	result.domain = key.domain();
	result.section = key.section();
	result.name = key.name();
	return result;
}

astro::config::ConfigurationKey	convert(const struct ConfigurationKey& key) {
	return astro::config::ConfigurationKey(key.domain, key.section,
		key.name);
}

struct ConfigurationItem	convert(const astro::config::ConfigurationEntry& entry) {
	struct ConfigurationItem	result;
	result.domain = entry.domain();
	result.section = entry.section();
	result.name = entry.name();
	result.value = entry.value();
	return result;
}

astro::config::ConfigurationEntry	convert(const struct ConfigurationItem& entry) {
	return astro::config::ConfigurationEntry(entry.domain, entry.section,
		entry.name, entry.value);
}

} // namespace snowstar
