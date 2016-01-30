/*
 * EventPersistence.cpp -- implementation of the event subsystem
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "AstroEvent.h"

using namespace astro::persistence;

namespace astro {
namespace event {

std::string	EventTableAdapter::tablename() {
	return std::string("events");
};

std::string	EventTableAdapter::createstatement() {
	return std::string(
		"create table events (\n"
		"    id integer not null,\n"
		"    pid integer not null,\n"
		"    service varchar(32) not null,\n"
		"    eventtime datetime not null,\n"
		"    subsystem varchar(32) not null,\n"
		"    message varchar(1024) not null,\n"
		"    object varchar(1024) not null,\n"
		"    file varchar(1024) not null,\n"
		"    line integer not null,\n"
		"    primary key(id)\n"
		")\n"
	);
}

EventRecord	EventTableAdapter::row_to_object(int objectid, const Row& row) {
	EventRecord	record(objectid);
	record.pid = row["pid"]->intValue();
	record.service = row["service"]->stringValue();
	record.eventtime = row["eventtime"]->timeValue();
	record.subsystem = row["subsystem"]->stringValue();
	record.message = row["message"]->stringValue();
	record.object = row["object"]->stringValue();
	record.file = row["file"]->stringValue();
	record.line = row["line"]->intValue();
	return record;
}

UpdateSpec	EventTableAdapter::object_to_updatespec(const EventRecord& event) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("pid", factory.get(event.pid)));
	spec.insert(Field("service", factory.get(event.service)));
	spec.insert(Field("eventtime", factory.getTime(event.eventtime)));
	spec.insert(Field("subsystem", factory.get(event.subsystem)));
	spec.insert(Field("message", factory.get(event.message)));
	spec.insert(Field("object", factory.get(event.object)));
	spec.insert(Field("file", factory.get(event.file)));
	spec.insert(Field("line", factory.get(event.line)));
	return spec;
}

} // namespace event
} // namespace astro
