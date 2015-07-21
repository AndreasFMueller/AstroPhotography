/*
 * ServerTable.h -- Table containing server information to simplify the
 *                  locating servers
 * 
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ServerTable_h
#define _ServerTable_h

#include <AstroPersistence.h>

namespace astro {
namespace config {

/**
 * \brief The server info holder class
 */
class ServerInfoData {
public:
	std::string	name;
	std::string	url;
	std::string	info;
};

/**
 * \brief server info record class
 */
class ServerRecord : public persistence::Persistent<ServerInfoData> {
public:
	ServerRecord(int id = -1) : persistence::Persistent<ServerInfoData>(id) { }
};

/**
 * \brief Adapter for the server information table
 */
class ServerTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static ServerRecord	row_to_object(int objectid,
				const persistence::Row& row);
static persistence::UpdateSpec	object_to_updatespec(const ServerRecord& serverinfo);
};

/**
 * \brief The server information table class
 */
class ServerTable : public persistence::Table<ServerRecord, ServerTableAdapter> {
public:
	ServerTable(persistence::Database database)
		: persistence::Table<ServerRecord,
			ServerTableAdapter>(database) {
	}
	long	id(const std::string& name);
};


} // namespace config
} // namespace astro

#endif /* _ServerTable_h */
