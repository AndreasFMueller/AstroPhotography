/*
 * ConfigurationTable.h --
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ConfigurationTable_h
#define _ConfigurationTable_h

#include <AstroPersistence.h>
#include <AstroFormat.h>
#include <AstroConfig.h>

using namespace astro::persistence;

namespace astro {
namespace config {

/**
 * \brief Wrapper around global configuration info, adds object id
 */
class ConfigurationRecord : public Persistent<ConfigurationEntry> {
public:
	ConfigurationRecord(int id = -1)
		: Persistent<ConfigurationEntry>(id) { }
	ConfigurationRecord(const ConfigurationEntry& entry, int id = -1)
		: Persistent<ConfigurationEntry>(id, entry) {
	}
};

/**
 * \brief Adapter for the global configuration table
 */
class ConfigurationTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static ConfigurationRecord	row_to_object(int objectid, const Row& row);
static UpdateSpec	object_to_updatespec(const ConfigurationRecord& global);
};

/**
 * \brief The global configuration information table
 */
class ConfigurationTable : public Table<ConfigurationRecord, ConfigurationTableAdapter> {
public:
	ConfigurationTable(Database database)
		: Table<ConfigurationRecord, ConfigurationTableAdapter>(
			database) { }

	// basic access based on keys
	long	key2id(const ConfigurationKey& key);
	ConfigurationRecord	operator[](const ConfigurationKey& key);
	void	remove(const ConfigurationKey& key);
	void	remove(const std::string& domain, const std::string& section,
			const std::string& name);

private:
	std::string	condition(const std::string& domain) const;
	std::string	condition(const std::string& domain,
					const std::string& section) const;
	std::string	condition(const std::string& domain,
					const std::string& section,
					const std::string& name) const;
	std::string	condition(const ConfigurationKey& key) const;

public:
	// list operators for lists of ids
	std::list<long>	list();
	std::list<long>	list(const std::string& domain);
	std::list<long>	list(const std::string& domain,
				const std::string& section);

	// list operators for lists of entries
	std::list<ConfigurationEntry>	selectAll();
	std::list<ConfigurationEntry>	selectDomain(const std::string& domain);
	std::list<ConfigurationEntry>	selectSection(const std::string& domain,
		const std::string& section);
};

} // namespace config
} // namespace astro


#endif /* _ConfigurationTable_h */
