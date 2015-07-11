/*
 * InstrumentComponentTable.cpp -- table for instrument components
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <InstrumentComponentTable.h>

namespace astro {
namespace discover {

std::string	InstrumentComponentTableAdapter::tablename() {
	return std::string("instrumentcomponents");
}

std::string	InstrumentComponentTableAdapter::createstatement() {
	return std::string(
		"create table instrumentcomponents (\n"
		"    id integer not null,\n"
		"    name varchar(32) not null,\n"
		"    type integer not null,\n"
		"    idx integer not null,\n"
		"    servicename varchar(128) not null,\n"
		"    deviceurl varchar(255) not null,\n"
		"    primary key(id)\n"
		");\n"
		"create unique index instrumentcomponents_idx1 "
			"on instrumentcomponents(name, type, idx);\n"
	);
}

InstrumentComponentRecord	InstrumentComponentTableAdapter::row_to_object(
					int objectid, const Row& row) {
	InstrumentComponentRecord	record(objectid);
	record.name(row["name"]->stringValue());
	record.type((InstrumentComponent::Type)row["type"]->intValue());
	record.index(row["idx"]->intValue());
	record.servicename(row["servicename"]->stringValue());
	record.deviceurl(row["deviceurl"]->stringValue());
	return record;
}

UpdateSpec	InstrumentComponentTableAdapter::object_to_updatespec(
			const InstrumentComponentRecord& component) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("name", factory.get(component.name())));
	spec.insert(Field("type", factory.get(component.type())));
	spec.insert(Field("idx", factory.get(component.index())));
	spec.insert(Field("servicename", factory.get(component.servicename())));
	spec.insert(Field("deviceurl", factory.get(component.deviceurl())));
	return spec;
}


} // namespace discover
} // namespace astro
