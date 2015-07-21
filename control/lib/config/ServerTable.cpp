/*
 * ServerTable.cpp -- implementation of the server info table access
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "ServerTable.h"
#include <AstroFormat.h>

using namespace astro::persistence;

namespace astro {
namespace config {

std::string	ServerTableAdapter::tablename() {
	return std::string("server");
}

std::string	ServerTableAdapter::createstatement() {
	return std::string(
		"create table server (\n"
		"    id integer not null,\n"
		"    name varchar(32) not null,\n"
		"    url varchar(1024) not null,\n"
		"    info varchar(1024) not null,\n"
		"    primary key(id)\n"
		");\n"
		"create unique index server_x1 on server(name);\n"
	);
}

ServerRecord	ServerTableAdapter::row_to_object(int objectid,
				const Row& row) {
	ServerRecord	record(objectid);
	record.name = row["name"]->stringValue();
	record.url = row["url"]->stringValue();
	record.info = row["info"]->stringValue();
	return record;
}

UpdateSpec	ServerTableAdapter::object_to_updatespec(const ServerRecord& serverinfo) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("name", factory.get(serverinfo.name)));
	spec.insert(Field("url", factory.get(serverinfo.url)));
	spec.insert(Field("info", factory.get(serverinfo.info)));
	return spec;
}

long	ServerTable::id(const std::string& name) {
	std::string	condition = stringprintf("name = '%s'",
				name.c_str());
	return TableBase::id(condition);
}

} // namespace config
} // namespace astro
