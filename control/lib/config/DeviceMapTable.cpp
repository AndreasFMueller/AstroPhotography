/*
 * DeviceMapTable.cpp -- implementation of the device map table
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "DeviceMapTable.h"

namespace astro {
namespace config {

std::string	DeviceMapTableAdapter::tablename() {
	return std::string("devicemap");
}

std::string	DeviceMapTableAdapter::createstatement() {
	return std::string(
		"create table devicemap (\n"
		"    id int not null,\n"
		"    name varchar(8) not null,\n"
		"    devicename varchar(128) not null,\n"
		"    unitid int not null default 0,\n"
		"    servername varchar(128),\n"
		"    description varchar(1024) not null default '',\n"
		"    primary key(id)\n"
		");\n"
		"create unique index devicemap_idx1 on devicemap(name);\n"
		"create unique index devicemap_idx2 on "
		"  devicemap(servername, devicename, unitid);\n"
	);
}

DeviceMapRecord	DeviceMapTableAdapter::row_to_object(int objectid, const Row& row) {
	DeviceMapRecord	result(objectid);
	result.name = row["name"]->stringValue();
	result.devicename = row["devicename"]->stringValue();
	result.unitid = row["unitid"]->intValue();
	result.servername = row["servername"]->stringValue();
	result.description = row["description"]->stringValue();
	return result;
}

UpdateSpec	DeviceMapTableAdapter::object_to_updatespec(const DeviceMapRecord& devicemap) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("name", factory.get(devicemap.name)));
	spec.insert(Field("devicename", factory.get(devicemap.devicename)));
	spec.insert(Field("unitid", factory.get(devicemap.unitid)));
	spec.insert(Field("servername", factory.get(devicemap.servername)));
	spec.insert(Field("description", factory.get(devicemap.description)));
	return spec;
}

} // namespace config
} // namespace astro
