/*
 * InstrumentPropertyTable.h -- table to store instrument properties
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _InstrumentPropertyTable_h
#define _InstrumentPropertyTable_h

#include <AstroPersistence.h>
#include <AstroDiscovery.h>

using namespace astro::persistence;

namespace astro {
namespace discover {

/**
 * \brief record class for the instrument property
 */
class InstrumentPropertyRecord : public Persistent<InstrumentProperty> {
public:
	InstrumentPropertyRecord(long id = -1)
		: Persistent<InstrumentProperty>(id) { }
	InstrumentPropertyRecord(const InstrumentProperty& property,
		long id = -1)
		: Persistent<InstrumentProperty>(property, id) { }
};

/** 
 * \brief Table adapter for the InstrumentProperty table
 */
class InstrumentPropertyTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static InstrumentPropertyRecord	row_to_object(int objectid, const Row& row);
static UpdateSpec	object_to_updatespec(
				const InstrumentPropertyRecord& property);
};

/**
 * \brief Instrument property table class
 */
class InstrumentPropertyTable : public Table<InstrumentPropertyRecord,
				InstrumentPropertyTableAdapter> {
public:
	InstrumentPropertyTable(Database& database)
		: Table<InstrumentPropertyRecord,
			InstrumentPropertyTableAdapter>(database) {
	}
};

typedef std::shared_ptr<InstrumentPropertyTable>	InstrumentPropertyTablePtr;

} // namespace discover
} // namespace astro

#endif /* _InstrumentPropertyTable_h */
