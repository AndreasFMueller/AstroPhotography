/*
 * GlobalTable.h --
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GlobalTable_h
#define _GlobalTable_h

#include <AstroPersistence.h>

using namespace astro::persistence;

namespace astro {
namespace config {

/**
 * \brief Global configuration info
 */
class GlobalInfo {
public:
	std::string	section;
	std::string	name;
	std::string	value;
};

/**
 * \brief Wrapper around global configuration info, adds object id
 */
class GlobalRecord : public Persistent<GlobalInfo> {
public:
	GlobalRecord(int id = -1) : Persistent<GlobalInfo>(id) { }
};

/**
 * \brief Adapter for the global configuration table
 */
class GlobalTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static GlobalRecord	row_to_object(int objectid, const Row& row);
static UpdateSpec	object_to_updatespec(const GlobalRecord& global);
};

/**
 * \brief The global configuration information table
 */
class GlobalTable : public Table<GlobalRecord, GlobalTableAdapter> {
public:
	GlobalTable(Database database)
		: Table<GlobalRecord, GlobalTableAdapter>(database) { }
};

} // namespace config
} // namespace astro


#endif /* _GlobalTable_h */
