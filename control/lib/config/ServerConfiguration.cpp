/*
 * ServerConfiguration.cpp -- backend for server configuration stuff
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConfig.h>
#include <AstroPersistence.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <cstdlib>
#include "ServerTable.h"

using namespace astro::persistence;
using namespace astro::project;

namespace astro {
namespace config {

/**
 * \brief configuration backend
 *
 * This is used to hide the fact that there 
 */
class ServerConfigurationBackend : public ServerConfiguration {
	ConfigurationPtr	_config;
public:
	ServerConfigurationBackend(ConfigurationPtr config) : _config(config) { 
	}
public:
	// access to server information
	virtual ServerInfo	server(const std::string& name);
	virtual void	addserver(const ServerInfo& server);
	virtual void	removeserver(const std::string& server);
	virtual std::list<ServerInfo>	listservers();
};

//////////////////////////////////////////////////////////////////////
// server information access
//////////////////////////////////////////////////////////////////////
ServerInfo	ServerConfigurationBackend::server(const std::string& name) {
	ServerTable	servers(_config->database());
	long	serverid = servers.id(name);
	ServerRecord	record = servers.byid(serverid);
	ServerInfo	si(record.name, ServerName(record.url));
	si.info(record.info);
	return si;
}

void	ServerConfigurationBackend::addserver(const ServerInfo& server) {
	ServerTable	servers(_config->database());
	ServerRecord	si(-1);
	si.name = server.name();
	si.url = (std::string)server.servername();
	si.info = server.info();
	servers.add(si);
}

void	ServerConfigurationBackend::removeserver(const std::string& name) {
	ServerTable	servers(_config->database());
	long	serverid = servers.id(name);
	servers.remove(serverid);
}

std::list<ServerInfo>	ServerConfigurationBackend::listservers() {
	std::list<ServerInfo>	result;
	ServerTable	servers(_config->database());
	std::list<ServerRecord>	rl = servers.select("0 = 0");
	for (auto ptr = rl.begin(); ptr != rl.end(); ptr++) {
		ServerInfo	si(ptr->name, ServerName(ptr->url));
		si.info(ptr->info);
		result.push_back(si);
	}
	return result;
}

//////////////////////////////////////////////////////////////////////
// ServerConfiguration implementation (static methods)
//////////////////////////////////////////////////////////////////////
ServerConfigurationPtr	ServerConfiguration::get() {
	return ServerConfigurationPtr(
			new ServerConfigurationBackend(Configuration::get()));
}

ServerConfigurationPtr	ServerConfiguration::get(ConfigurationPtr config) {
	return ServerConfigurationPtr(new ServerConfigurationBackend(config));
}

} // namespace config
} // namespace astro
