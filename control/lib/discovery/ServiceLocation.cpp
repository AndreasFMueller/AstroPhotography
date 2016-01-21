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

/**
 * \brief locate a service
 */
void	ServiceLocation::locate() {
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	if (_servicename.size() == 0) {
		if (config->hasglobal("service", "name")) {
			_servicename = config->global("service", "name");
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
		if (config->hasglobal("service", "port")) {
			std::string	s = config->global("service", "port");
			_port = std::stoi(s);
		} else {
			_port = 10000;
		}
	}
	if (_sslport == 0) {
		if (config->hasglobal("service", "sslport")) {
			std::string s = config->global("service", "sslport");
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
