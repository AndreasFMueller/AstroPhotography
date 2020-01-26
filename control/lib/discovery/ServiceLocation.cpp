/*
 * ServiceLocation.cpp -- class to locate a service
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDiscovery.h>
#include <AstroConfig.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <includes.h>

namespace astro {
namespace discover {

static config::ConfigurationKey	_name_key("global", "service", "name");
static config::ConfigurationRegister	_name_registration(_name_key,
	"name of the service");

static config::ConfigurationKey	_port_key("global", "service", "port");
static config::ConfigurationRegister	_port_registration(_port_key,
	"port for the service");

static config::ConfigurationKey	_sslport_key("global", "service", "sslport");
static config::ConfigurationRegister	_sslport_registration(_sslport_key,
	"port for the SSL encrypted  service");

/**
 * \brief locate a service
 */
void	ServiceLocation::locate() {
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	if (_servicename.size() == 0) {
		if (config->has(_name_key)) {
			_servicename = config->get(_name_key);
		} else {
			char	h[1024];
			if (gethostname(h, sizeof(h)) < 0) {
				std::string	msg = astro::stringprintf(
					"cannot figure out host name: %s",
					strerror(errno));
				debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
				throw std::runtime_error(msg);
			}
			_servicename = std::string(h);
		}
	}
	if (_port == 0) {
		if (config->has(_port_key)) {
			std::string	s = config->get(_port_key);
			_port = std::stoi(s);
		} else {
			_port = 10000;
		}
	}
	if (_sslport == 0) {
		if (config->has(_sslport_key)) {
			std::string s = config->get(_sslport_key);
			_sslport = std::stoi(s);
		}
	}
	_ssl = (_sslport > 0);
}

static ServiceLocation	ourLocation;
static std::once_flag	ready;
static void	setup() {
	ourLocation.locate();
}

/**
 * \brief Access to the singleton location object
 */
ServiceLocation&	ServiceLocation::get() {
	// run locate once
	std::call_once(ready, setup);

	// return a reference to our location
	return ourLocation;
}

} // namespace discover
} // namespace astro
