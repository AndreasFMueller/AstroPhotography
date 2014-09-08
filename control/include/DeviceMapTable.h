/*
 * DeviceMapTable.h -- 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DeviceMapTable_h
#define _DeviceMapTable_h

#include <AstroPersistence.h>

using namespace astro::persistence;

namespace astro {
namespace config {

class DeviceMapInfo {
public:
	std::string	name;
	std::string	devicename;
	std::string	servername;
	std::string	description;
};

class DeviceMapRecord : public Persistent<DeviceMapInfo> {
public:
	DeviceMapRecord(int id = -1) : Persistent<DeviceMapInfo>(id) { }
};

class DeviceMapTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static DeviceMapRecord	row_to_object(int objectid, const Row& row);
static UpdateSpec	object_to_updatespec(const DeviceMapRecord& devicemap);
};

class DeviceMapTable : public Table<DeviceMapRecord, DeviceMapTableAdapter> {
public:
	DeviceMapTable(Database database)
		: Table<DeviceMapRecord, DeviceMapTableAdapter>(database) {
	}
};

} // namespace config
} // namespace astro

#endif /* _DeviceMapTable_h */
