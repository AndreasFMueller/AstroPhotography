/*
 * InstrumentPropertyTable.cpp --
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <InstrumentPropertyTable.h>
#include <AstroDebug.h>

namespace astro {
namespace discover {

//////////////////////////////////////////////////////////////////////
// Instrument Property table adaapter implementation
//////////////////////////////////////////////////////////////////////
std::string	InstrumentPropertyTableAdapter::tablename() {
			return std::string("instrumentproperties");
}

std::string	InstrumentPropertyTableAdapter::createstatement() {
	return std::string(
		"create table instrumentproperties (\n"
		"    id integer not null,\n"
		"    instrument varchar(32) not null,\n"
		"    property varchar(256) not null,\n"
		"    value varchar(1024) not null,\n"
		"    description varchar(1024) not null,\n"
		"    primary key(id)\n"
		");\n");
}

InstrumentPropertyRecord	InstrumentPropertyTableAdapter::row_to_object(int objectid, const Row& row) {
	InstrumentPropertyRecord	record(objectid);
	record.instrument(row["instrument"]->stringValue());
	record.property(row["property"]->stringValue());
	record.value(row["value"]->stringValue());
	record.description(row["description"]->stringValue());
	return record;
}

UpdateSpec	InstrumentPropertyTableAdapter::object_to_updatespec(
			const InstrumentPropertyRecord& record) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("instrument", factory.get(record.instrument())));
	spec.insert(Field("property", factory.get(record.property())));
	spec.insert(Field("value", factory.get(record.value())));
	spec.insert(Field("description", factory.get(record.description())));
	return spec;
}

} // namespace discover
} // namespace stro
