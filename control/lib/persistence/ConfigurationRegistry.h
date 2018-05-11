/*
 * ConfigurationRegistry.h -- registry of defined keys
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _ConfigurationRegistry_h
#define _ConfigurationRegistry_h

#include <AstroConfig.h>
#include <map>

namespace astro {
namespace config {

/**
 * \brief common class used to register known registration keys
 */
class ConfigurationRegistry {
	typedef std::map<ConfigurationKey, std::string>	descriptions_t;
	descriptions_t	_descriptions;
public:
	ConfigurationRegistry() { }
	~ConfigurationRegistry() { }

	void	add(const ConfigurationKey& key,
			const std::string& description);

	void	add(const std::string& domain, const std::string& section,
			const std::string& name,
			const std::string& description) {
		add(ConfigurationKey(domain, section, name), description);
	}

	std::list<ConfigurationKey>	list() const;

	const std::string&	describe(const ConfigurationKey& key) const;

	const std::string&	describe(const std::string& domain,
					const std::string& section,
					const std::string& name) const {
		return describe(ConfigurationKey(domain, section, name));
	}

	const std::string&	operator()(const ConfigurationKey& key) const {
		return describe(key);
	}

	const std::string&	operator()(const std::string& domain,
					const std::string& section,
					const std::string& name) const {
		return describe(ConfigurationKey(domain, section, name));
	}

	void	show(std::ostream& out, bool showdescriptions = false) const;
};

typedef std::shared_ptr<ConfigurationRegistry>	ConfigurationRegistryPtr;

} // namespace config
} // namespace astro

#endif /* _ConfigurationRegistry_h */
