/*
 * DeviceMapperConfiguration.cpp -- device mapper backend
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConfig.h>
#include <AstroPersistence.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <cstdlib>

using namespace astro::persistence;
using namespace astro::project;

namespace astro {
namespace config {

/**
 * \brief configuration backend
 *
 * This is used to hide the fact that there 
 */
class DeviceMapperConfigurationBackend : public DeviceMapperConfiguration {
	ConfigurationPtr	_config;
public:
	DeviceMapperConfigurationBackend(ConfigurationPtr config)
		: _config(config) { }
	virtual ~DeviceMapperConfigurationBackend() { }

	// devicemapper access
	virtual DeviceMapperPtr	devicemapper();
};

//////////////////////////////////////////////////////////////////////
// DeviceMapperConfiguration implementation (static methods)
//////////////////////////////////////////////////////////////////////
DeviceMapperConfigurationPtr	DeviceMapperConfiguration::get() {
	return DeviceMapperConfigurationPtr(
		new DeviceMapperConfigurationBackend(Configuration::get()));
}

DeviceMapperConfigurationPtr	DeviceMapperConfiguration::get(ConfigurationPtr config) {
	return DeviceMapperConfigurationPtr(
		new DeviceMapperConfigurationBackend(config));
}

//////////////////////////////////////////////////////////////////////
// device mapper access
//////////////////////////////////////////////////////////////////////
/**
 * \brief Get the device mapper
 */
DeviceMapperPtr	DeviceMapperConfigurationBackend::devicemapper() {
	return DeviceMapper::get(_config->database());
}

} // namespace config
} // namespace astro
