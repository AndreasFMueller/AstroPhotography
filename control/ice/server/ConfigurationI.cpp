/*
 * ConfigurationI.cpp -- configuration servant implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "ConfigurationI.h"
#include <AstroConfig.h>
#include <AstroDebug.h>
#include <AstroFormat.h>

namespace snowstar {

ConfigurationI::ConfigurationI(astro::config::ConfigurationPtr _configuration)
	: configuration(_configuration) {
}

bool	ConfigurationI::has(const ConfigurationKey& key,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "check whether get %s.%s.%s exists",
		key.domain.c_str(), key.section.c_str(), key.name.c_str());
	if (key.domain != "global") {
		return false;
	}
	return configuration->hasglobal(key.section, key.name);
}

ConfigurationItem	ConfigurationI::get(const ConfigurationKey& key,
				const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get configuration %s.%s.%s",
		key.domain.c_str(), key.section.c_str(), key.name.c_str());
	if (key.domain != "global") {
		std::string	msg = astro::stringprintf("domain %s not "
			"implemented", key.domain.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}
	if (!configuration->hasglobal(key.section, key.name)) {
		std::string	msg = astro::stringprintf("section=%s, name=%s "
			"not found", key.section.c_str(), key.name.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}
	ConfigurationItem	entry;
	entry.domain = "global";
	entry.section = key.section;
	entry.name = key.name;
	entry.value = configuration->global(key.section, key.name);
	return entry;
}

void	ConfigurationI::set(const ConfigurationItem& item,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set configuration %s.%s.%s",
		item.domain.c_str(), item.section.c_str(), item.name.c_str());
	if (item.domain != "global") {
		throw BadParameter("only global domain implemented");
	}
	configuration->setglobal(item.section, item.name, item.value);
}

ConfigurationList	ConfigurationI::list(const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "list all config variables");
	return listDomain("global", current);
}

ConfigurationList	ConfigurationI::listDomain(const std::string& domain,
				const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "list domain %s", domain.c_str());
	ConfigurationList	result;
	if (domain != "global") {
		return result;
	}
	std::list<astro::config::ConfigurationEntry>	l
		= configuration->globallist();
	std::for_each(l.begin(), l.end(),
		[&result](const astro::config::ConfigurationEntry& e) {
			ConfigurationItem	entry;
			entry.domain = "global";
			entry.section = e.section;
			entry.name = e.name;
			entry.value = e.value;
			result.push_back(entry);
		}
	);
	return result;
}

ConfigurationList	ConfigurationI::listSection(const std::string& domain,
				const std::string& section,
				const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "list section %s.%s", domain.c_str(),
		section.c_str());
	ConfigurationList	result;
	if (domain != "global") {
		return result;
	}
	std::list<astro::config::ConfigurationEntry>	l
		= configuration->globallist();
	std::for_each(l.begin(), l.end(),
		[&result,section](const astro::config::ConfigurationEntry& e) {
			if (e.section != section) {
				return;
			}
			ConfigurationItem	entry;
			entry.domain = "global";
			entry.section = e.section;
			entry.name = e.name;
			entry.value = e.value;
			result.push_back(entry);
		}
	);
	return result;
}

} // namespace snowstar
