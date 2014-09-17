/*
 * InstrumentTables.h -- Tables for instrument configuration information
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _InstrumentTables_h
#define _InstrumentTables_h

#include <AstroPersistence.h>
#include <AstroDevice.h>
#include <AstroConfig.h>

using namespace astro::persistence;
using namespace astro::device;

namespace astro {
namespace config {

/**
 * \brief basic Instrument Information abstraction
 */
class InstrumentInfo {
public:
	std::string	name;
};

/**
 * \brief Instrument Record
 *
 * The InsrumentRecord adds the instrument identifier to the InstrumentInfo
 */
class InstrumentRecord : public Persistent<InstrumentInfo> {
public:
	InstrumentRecord(long id = -1) : Persistent<InstrumentInfo>(id) { }
};

/**
 * \brief Table adapter for the instrument table
 *
 *
 */
class InstrumentTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static InstrumentRecord	row_to_object(int objectid, const Row& row);
static UpdateSpec object_to_updatespec(const InstrumentRecord& instrumentinfo);
};

/**
 * \brief The Instrument table
 */
class InstrumentTable : public Table<InstrumentRecord, InstrumentTableAdapter> {
public:
	InstrumentTable(Database database)
		: Table<InstrumentRecord, InstrumentTableAdapter>(database) {
	}
	long	id(const std::string& name);
};

/**
 * \brief abstraction of component information
 */
class InstrumentComponentInfo {
public:
	std::string	type;
	std::string	componenttype;
	std::string	devicename;
	int	unit;
	std::string	servername;
};

/**
 * \brief component record adds the component id and the reference field
 */
class InstrumentComponentRecord : public PersistentRef<InstrumentComponentInfo> {
public:
	InstrumentComponentRecord(int id, int instrumentid)
		: PersistentRef<InstrumentComponentInfo>(id, instrumentid) {
	}
	int	instrumentid() const { return ref(); }
};

/**
 * \brief Table adapter for component table
 */
class InstrumentComponentTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static InstrumentComponentRecord
	row_to_object(int objectid, const Row& row);
static UpdateSpec
	object_to_updatespec(const InstrumentComponentRecord& component);

static std::string	type(DeviceName::device_type t);
static DeviceName::device_type	type(const std::string& t);

static std::string	component_type(InstrumentComponent::component_t c);
static InstrumentComponent::component_t	component_type(const std::string& s);
};

/**
 * \brief The component table
 */
class InstrumentComponentTable : public Table<InstrumentComponentRecord, InstrumentComponentTableAdapter> {
public:
	InstrumentComponentTable(Database& database)
		: Table<InstrumentComponentRecord,
			InstrumentComponentTableAdapter>(database) {
	}
};

} // namespace project
} // namespace astro

#endif /* _InstrumentTables_h */
