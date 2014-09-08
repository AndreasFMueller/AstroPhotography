/*
 * GlobalTable.cpp -- global configuration variables table
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GlobalTable.h>

namespace astro {
namespace config {

std::string	GlobalTableAdapter::tablename() {
	return std::string("global");
}

std::string	GlobalTableAdapter::createstatement() {
	return std::string(
		"create table global (\n"
		"    id int not null,\n"
		"    section varchar(128) not null default '.',\n"
		"    name varchar(128) not null,\n"
		"    value varchar(1024) not null,\n"
		"    primary key(id)\n"
		");\n"
		"create unique index global_idx1 on global(section, name);\n"
	);
}

GlobalRecord	GlobalTableAdapter::row_to_object(int objectid, const Row& row) {
	GlobalRecord	record(objectid);
	record.section = row["section"]->stringValue();
	record.name = row["name"]->stringValue();
	record.value = row["value"]->stringValue();
	return record;
}

UpdateSpec	GlobalTableAdapter::object_to_updatespec(const GlobalRecord& global) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("section", factory.get(global.section)));
	spec.insert(Field("name", factory.get(global.name)));
	spec.insert(Field("value", factory.get(global.value)));
	return spec;
}

} // namespace config
} // namespace astro
