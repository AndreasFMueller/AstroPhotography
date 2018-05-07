/*
 * Configuration.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConfig.h>
#include <AstroPersistence.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <cstdlib>
#include "ConfigurationBackend.h"

using namespace astro::persistence;
using namespace astro::project;

namespace astro {
namespace config {

//////////////////////////////////////////////////////////////////////
// implementation of the static methods of the Configuration fact
//////////////////////////////////////////////////////////////////////

typedef std::map<std::string, ConfigurationPtr>	configurationmap_t;
static configurationmap_t	configurationmap;

static std::string	configfilename() {
	std::string	filename;
	// get the path in the home directory
	char	*home = getenv("HOME");
	if (NULL != home) {
		filename = std::string(home) + "/.astrophoto.db";
	}

	// first check whether the environment variable is set
	char	*apconfig = getenv("AstroPhotoConfig");
	if (NULL != apconfig) {
		filename = std::string(apconfig);
	}
	return filename;
}

/**
 * \brief Get the configuration
 */
ConfigurationPtr	Configuration::get() {
	std::string	filename = get_default();

	// if the filename is still empty, then we have a problem
	debug(LOG_DEBUG, DEBUG_LOG, 0, "configuration file: %s",
		filename.c_str());
	if (0 == filename.size()) {
		throw std::runtime_error("no default config file name found");
	}
	return get(filename);
}

/**
 * \brief Get the configuration stored in a given database
 */
ConfigurationPtr	Configuration::get(const std::string& filename) {
	// check whether the configuration is already in the map
	configurationmap_t::iterator	i = configurationmap.find(filename);
	if (i != configurationmap.end()) {
		return i->second;
	}

	// we need to create a new configuration
	ConfigurationPtr	config(new ConfigurationBackend(filename));
	configurationmap.insert(std::make_pair(filename, config));
	return config;
}

static std::string	default_config;

/**
 * \brief get the default configuration filename
 */
std::string	Configuration::get_default() {
	if (0 == default_config.size()) {
		default_config = configfilename();
	}
	return default_config;
}

/**
 * \brief set the default filename
 */
void	Configuration::set_default(const std::string& filename) {
	default_config = filename;
}

/**
 * \brief Get the system database
 */
persistence::Database	Configuration::systemdatabase() {
	return database();
}

} // namespace config
} // namespace astro
