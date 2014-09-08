/*
 * AstroConfig.h -- classes for configuration management of the AP application
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroConfig_h
#define _AstroConfig_h

#include <memory>

namespace astro {
namespace config {

class Configuration;
typedef std::shared_ptr<Configuration>	ConfigurationPtr;

/**
 * \brief Configuration repository class
 *
 * All configuration information can be accessed through this interface
 */
class Configuration {
private: // prevent copying of configuration
	Configuration(const Configuration& other);
	Configuration&	operator=(const Configuration& other);
public:
	Configuration() { }
	// factory methods for the configuration
static ConfigurationPtr	get();
static ConfigurationPtr	get(const std::string& filename);
	// global configuration variables
	virtual std::string	global(const std::string& section,
				const std::string& name) = 0;
	virtual std::string	global(const std::string& section,
				const std::string& name,
				const std::string& def) = 0;
	virtual void	setglobal(const std::string& section,
			const std::string& name, const std::string& value) = 0;
	virtual void	removeglobal(const std::string& section,
				const std::string& name) = 0;
};

} // namespace config
} // namespace astro

#endif /* _AstroConfig_h */
