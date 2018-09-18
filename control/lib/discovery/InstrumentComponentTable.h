/*
 * InstrumentComponentTable.h -- Tables for InstrumentComponents
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _InstrumentComponentTable_h
#define _InstrumentComponentTable_h

#include <AstroDiscovery.h>
#include <AstroPersistence.h>

using namespace astro::persistence;

namespace astro {
namespace discover {

/**
 * \brief Component info class
 *
 * This class represents the information in an instrument component 
 */
class InstrumentComponentInfo : public InstrumentComponentKey {
	std::string	_servicename;
	std::string	_deviceurl;
public:
	const std::string&	servicename() const { return _servicename; }
	std::string&	servicename() { return _servicename; }
	void	servicename(const std::string& s) { _servicename = s; }

	const std::string&	deviceurl() const { return _deviceurl; }
	std::string&	deviceurl() { return _deviceurl; }
	void	deviceurl(const std::string& d) { _deviceurl = d; }

	InstrumentComponentInfo(const InstrumentComponentKey& key)
		: InstrumentComponentKey(key) {
	}
	InstrumentComponentInfo(const InstrumentComponent& component)
		: InstrumentComponentKey(component),
		  _servicename(component.servicename()),
		  _deviceurl(component.deviceurl()) {
	}
	InstrumentComponentInfo() { }
};

/**
 * \brief Record definition for the instrument component table
 *
 * The record is what the table interface uses 
 */
class InstrumentComponentRecord : public Persistent<InstrumentComponentInfo> {
public:
	InstrumentComponentRecord(long id = -1)
		: Persistent<InstrumentComponentInfo>(id) { }
	InstrumentComponentRecord(const InstrumentComponentInfo& component,
		long id = -1)
		: Persistent<InstrumentComponentInfo>(component, id) { }
};

/**
 * \brief Table adapter for the InstrumentComponent Table
 *
 * The table adapter for the instrument components table provides the
 * create statement and the methods to convert objects to updates and vice
 * versa
 */
class InstrumentComponentTableAdapter {
public:
static std::string      tablename();
static std::string      createstatement();
static InstrumentComponentRecord        row_to_object(int objectid,
						const Row& row);
static UpdateSpec       object_to_updatespec(
	const InstrumentComponentRecord& component);
};

/**
 * \brief InstrumentComponent Table
 *
 * The Table class for the InstrumentComponentTable gives access to the 
 * objects in the table
 */
class InstrumentComponentTable : public Table<InstrumentComponentRecord,
	InstrumentComponentTableAdapter> {
public:
        InstrumentComponentTable(Database& database)
                : Table<InstrumentComponentRecord,
                        InstrumentComponentTableAdapter>(database) {
        }
	virtual ~InstrumentComponentTable() { }
};
typedef std::shared_ptr<InstrumentComponentTable> InstrumentComponentTablePtr;

} // namespace discover
} // namespace astro

#endif /* _InstrumentComponentTable_h */
