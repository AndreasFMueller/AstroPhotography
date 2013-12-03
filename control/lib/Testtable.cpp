/*
 * Testtable.cpp -- auxiliary classes for test table
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Testtable.h>
#include <AstroDebug.h>

namespace astro {
namespace persistence {

std::ostream&	operator<<(std::ostream& out, const TestEntry& entry) {
	out << "id=" << entry.id() << " ";
	out << "intfield=" << entry.intfield() << " ";
	out << "floatfield=" << entry.doublefield() << " ";
	out << "stringfield='" << entry.stringfield() << "'";
	return out;
}

std::string	TesttableAdapter::tablename() {
	return std::string("testtable");
}

std::string	TesttableAdapter::createstatement() {
	return std::string(
	"create table testtable ("
	"    id integer not null,"
	"    intfield integer not null default 0,"
	"    floatfield float not null default 0,"
	"    stringfield varchar(256) not null default '',"
	"    primarykey(id)"
	")"
	);
}

TestEntry	TesttableAdapter::row_to_object(int objectid, const Row& row) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "objectid: %d", objectid);
	TestEntry	entry(objectid);
	entry.intfield(row["intfield"]->intValue());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "intfield = %d",
		entry.intfield());
	entry.doublefield(row["floatfield"]->doubleValue());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "doublefield = %f",
		entry.doublefield());
	entry.stringfield(row["stringfield"]->stringValue());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stringfield = %s",
		entry.stringfield().c_str());
	return entry;
}

UpdateSpec	TesttableAdapter::object_to_updatespec(const TestEntry& entry) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("intfield", factory.get(entry.intfield())));
	spec.insert(Field("floatfield", factory.get(entry.doublefield())));
	spec.insert(Field("stringfield", factory.get(entry.stringfield())));
	return spec;
}

} // namespace persistence
} // namespace astro
