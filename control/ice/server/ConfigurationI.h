/*
 * ConfigurationI.h -- configuration servant definition 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ConfigurationI_h
#define _ConfigurationI_h

#include <types.h>
#include <AstroConfig.h>
#include "StatisticsI.h"

namespace snowstar {

class ConfigurationI : virtual public Configuration, public StatisticsI {
	astro::config::ConfigurationPtr	configuration;
public:
	ConfigurationI(astro::config::ConfigurationPtr configuration);

	bool	has(const ConfigurationKey& key, const Ice::Current& current);
	ConfigurationItem	get(const ConfigurationKey& key,
					const Ice::Current& current);
	void	remove(const ConfigurationKey& key,
			const Ice::Current& current);
	void	set(const ConfigurationItem& item, const Ice::Current& current);
	ConfigurationList	list(const Ice::Current& current);
	ConfigurationList	listDomain(const std::string& domain,
					const Ice::Current& current);
	ConfigurationList	listSection(const std::string& domain,
					const std::string& section,
					const Ice::Current& current);
	void	shutdownServer(Ice::Float delay, const Ice::Current& current);
	void	restartServer(Ice::Float delay, const Ice::Current& current);
	ConfigurationKeyList	registeredKeys(const Ice::Current& current);
	std::string	description(const ConfigurationKey& key,
			const Ice::Current& current);
};

} // namespace snowstar

#endif /* _ConfigurationI_h */
