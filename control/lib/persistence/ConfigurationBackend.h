/*
 * ConfigurationBackend.h -- Database Backend for configuration
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ConfigurationBackend_h
#define _ConfigurationBackend_h

#include <AstroConfig.h>
#include <AstroPersistence.h>
#include <ConfigurationTable.h>

using namespace astro::persistence;
using namespace astro::project;

namespace astro {
namespace config {

/**
 * \brief configuration backend
 *
 * This is used to hide the fact that there 
 */
class ConfigurationBackend : public Configuration {
	std::string	_dbfilename;
	Database	_database;
	ConfigurationTable	_configurationtable;
public:
	// constructor
	ConfigurationBackend(const std::string& filename);
	// global configuratoin variables
	virtual bool	hasglobal(const std::string& section,
				const std::string& name);
	virtual std::string	global(const std::string& section,
					const std::string& name);
	virtual std::string	global(const std::string& section,
					const std::string& name,
					const std::string& def);
	virtual void	setglobal(const std::string& section,
			const std::string& name, const std::string& value);
	virtual void	removeglobal(const std::string& name,
				const std::string& value);
	virtual std::list<ConfigurationEntry>	globallist();

public:
	// all configuration variables
	virtual bool	has(const ConfigurationKey& key);
	virtual bool    has(const std::string& domain,
				const std::string& section,
				const std::string& name);
	virtual std::string	get(const ConfigurationKey& key);
	virtual std::string	get(const ConfigurationKey& key,
					const std::string& def);
	virtual std::string     get(const std::string& domain,
					const std::string& section,
					const std::string& name);
	virtual std::string     get(const std::string& domain,
					const std::string& section,
					const std::string& name,
					const std::string& def);
	virtual void    set(const std::string& domain,
				const std::string& section,
				const std::string& name,
				const std::string& value);
	virtual void	remove(const ConfigurationKey& key);
	virtual void    remove(const std::string& domain,
				const std::string& section,
				const std::string& name);
private:
	std::string	condition(const std::string& domain) const;
	std::string	condition(const std::string& domain,
				const std::string& section) const;
	std::string	condition(const std::string& domain,
				const std::string& section,
				const std::string& name) const;
public:
	virtual std::list<ConfigurationEntry>   list();
	virtual std::list<ConfigurationEntry>   list(const std::string& domain);
	virtual std::list<ConfigurationEntry>   list(const std::string& domain,
		const std::string& section);

	// get the configuration database
	virtual Database	database();
};

} // namespace config
} // namespace astro

#endif /* _ConfigurationBackend_h */
