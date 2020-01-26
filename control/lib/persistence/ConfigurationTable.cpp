/*
 * ConfigurationTable.cpp -- global configuration variables table
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ConfigurationTable.h>

namespace astro {
namespace config {

//////////////////////////////////////////////////////////////////////
// ConfigurationTableAdapter implementation
//////////////////////////////////////////////////////////////////////

std::string	ConfigurationTableAdapter::tablename() {
	return std::string("configuration");
}

std::string	ConfigurationTableAdapter::createstatement() {
	return std::string(
		"create table configuration (\n"
		"    id int not null,\n"
		"    domain varchar(128) not null default 'global',\n"
		"    section varchar(128) not null default '.',\n"
		"    name varchar(128) not null,\n"
		"    value varchar(1024) not null,\n"
		"    primary key(id)\n"
		");\n"
		"create unique index configuration_idx1 on"
		"    configuration(domain, section, name);\n"
	);
}

ConfigurationRecord	ConfigurationTableAdapter::row_to_object(int objectid,
				const Row& row) {
	ConfigurationEntry	entry(	row["domain"]->stringValue(),
					row["section"]->stringValue(),
					row["name"]->stringValue(),
					row["value"]->stringValue());
	ConfigurationRecord	record(entry, objectid);
	return record;
}

UpdateSpec	ConfigurationTableAdapter::object_to_updatespec(
			const ConfigurationRecord& configrec) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("domain", factory.get(configrec.domain())));
	spec.insert(Field("section", factory.get(configrec.section())));
	spec.insert(Field("name", factory.get(configrec.name())));
	spec.insert(Field("value", factory.get(configrec.value())));
	return spec;
}

//////////////////////////////////////////////////////////////////////
//  implementation of the ConfigurationTable class
//////////////////////////////////////////////////////////////////////
std::string	ConfigurationTable::condition(const std::string& domain) const {
	return stringprintf("domain = '%s'", domain.c_str());
}

std::string	ConfigurationTable::condition(const std::string& domain,
			const std::string& section) const {
	return stringprintf("domain = '%s' and section = '%s'",
		domain.c_str(), section.c_str());
}

std::string	ConfigurationTable::condition(const std::string& domain,
			const std::string& section,
			const std::string& name) const {
	return stringprintf("domain = '%s' and section = '%s' and name = '%s'",
		domain.c_str(), section.c_str(), name.c_str());
}

std::string	ConfigurationTable::condition(const ConfigurationKey& key) const {
	return condition(key.domain(), key.section(), key.name());
}

long	ConfigurationTable::key2id(const ConfigurationKey& key) {
	std::list<long>	ids = selectids(condition(key));
	if (ids.size() != 1) {
		std::string	msg = stringprintf("%s not found",
			key.toString().c_str());
		throw NoSuchEntry(msg);
	}
	return *ids.begin();
}

ConfigurationRecord	ConfigurationTable::operator[](const ConfigurationKey& key) {
	return byid(key2id(key));
}

void	ConfigurationTable::remove(const ConfigurationKey& key) {
	TableBase::remove(key.condition());
}

void	ConfigurationTable::remove(const std::string& domain, const std::string& section, const std::string& name) {
	remove(ConfigurationKey(domain, section, name));
}

std::list<long>	ConfigurationTable::list() {
	return selectids("0 = 0");
}

std::list<long>	ConfigurationTable::list(const std::string& domain) {
	return selectids(condition(domain));
}

std::list<long>	ConfigurationTable::list(const std::string& domain, 
                                const std::string& section) {
	return selectids(condition(domain, section));
}

std::list<ConfigurationEntry>	ConfigurationTable::selectAll() {
	return ObjectList<ConfigurationEntry, ConfigurationRecord>(
		select("0 = 0"));
}

std::list<ConfigurationEntry>	ConfigurationTable::selectDomain(
	const std::string& domain) {
	return ObjectList<ConfigurationEntry, ConfigurationRecord>(
		select(condition(domain)));
}

std::list<ConfigurationEntry>	ConfigurationTable::selectSection(
	const std::string& domain, const std::string& section) {
	return ObjectList<ConfigurationEntry, ConfigurationRecord>(
		select(condition(domain, section)));
}

} // namespace config
} // namespace astro
