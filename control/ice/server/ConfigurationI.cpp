/*
 * ConfigurationI.cpp -- configuration servant implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "ConfigurationI.h"
#include <AstroConfig.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <IceConversions.h>
#include <CommunicatorSingleton.h>
#include <thread>
#include "Restart.h"

namespace snowstar {

ConfigurationI::ConfigurationI(astro::config::ConfigurationPtr _configuration)
	: configuration(_configuration) {
}

bool	ConfigurationI::has(const ConfigurationKey& key,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "check whether get %s.%s.%s exists",
		key.domain.c_str(), key.section.c_str(), key.name.c_str());
	return configuration->has(convert(key));
}

ConfigurationItem	ConfigurationI::get(const ConfigurationKey& key,
				const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get configuration %s.%s.%s",
		key.domain.c_str(), key.section.c_str(), key.name.c_str());
	if (!configuration->has(convert(key))) {
		std::string	msg = astro::stringprintf("section=%s, name=%s "
			"not found", key.section.c_str(), key.name.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}
	astro::config::ConfigurationKey	ckey = convert(key);
	std::string	value = configuration->get(ckey);
	return convert(astro::config::ConfigurationEntry(ckey, value));
}

void	ConfigurationI::remove(const ConfigurationKey& key,
				const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove configuration %s.%s.%s",
		key.domain.c_str(), key.section.c_str(), key.name.c_str());
	if (!configuration->has(convert(key))) {
		std::string	msg = astro::stringprintf("section=%s, name=%s "
			"not found", key.section.c_str(), key.name.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}
	astro::config::ConfigurationKey	ckey = convert(key);
	configuration->remove(ckey);
}

void	ConfigurationI::set(const ConfigurationItem& item,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set configuration %s.%s.%s",
		item.domain.c_str(), item.section.c_str(), item.name.c_str());
	configuration->set(item.domain, item.section, item.name, item.value);
}

ConfigurationList	ConfigurationI::list(const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "list all config variables");
	return listDomain("global", current);
}

ConfigurationList	ConfigurationI::listDomain(const std::string& domain,
				const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "list domain %s", domain.c_str());
	ConfigurationList	result;
	std::list<astro::config::ConfigurationEntry>	l
		= configuration->list(domain);
	std::for_each(l.begin(), l.end(),
		[&result](const astro::config::ConfigurationEntry& e) {
			result.push_back(convert(e));
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
	std::list<astro::config::ConfigurationEntry>	l
		= configuration->list(domain, section);
	std::for_each(l.begin(), l.end(),
		[&result](const astro::config::ConfigurationEntry& e) {
			result.push_back(convert(e));
		}
	);
	return result;
}

static void	do_shutdown(float delay, const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "shutting down communicator in %f",
		delay);
	useconds_t	udelay = 1000000 * delay;
	usleep(udelay);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "shutting down communicator now");
	current.adapter->getCommunicator()->shutdown();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "shutdown complete");
}

void	ConfigurationI::shutdownServer(Ice::Float delay,
		const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "server shutdown requested");
	Restart::shutdown_instead(true);
	std::thread	*t = new std::thread(do_shutdown, delay, current);
}

void	ConfigurationI::restartServer(Ice::Float delay,
		const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "server restart requested");
	Restart::shutdown_instead(false);
	std::thread	*t = new std::thread(do_shutdown, delay, current);
}

} // namespace snowstar
