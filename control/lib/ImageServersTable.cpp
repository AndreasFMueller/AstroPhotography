/*
 * ImageServersTable.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ImageServersTable.h>
#include <ImageServerTables.h>
#include <AstroFormat.h>
#include <cassert>

namespace astro {
namespace project {

bool	ImageServerInfo::operator==(const ImageServerInfo& other) const {
	if (servername != other.servername) { return false; }
	if (database != other.database) { return false; }
	if (directory != other.directory) { return false; }
	return true;
}

bool	ImageServerRecord::operator==(const ImageServerRecord& other) const {
	if (id() != other.id()) { return false; }
	if (servername != other.servername) { return false; }
	if (database != other.database) { return false; }
	if (directory != other.directory) { return false; }
	return true;
}

std::string	ImageServerTableAdapter::tablename() {
	return std::string("imageservers");
}

std::string	ImageServerTableAdapter::createstatement() {
	return std::string(
		"create table imageservers (\n"
		"    id int not null,\n"
		"    servername varchar(32) not null,\n"
		"    dbname varchar(1024) not null,\n"
		"    directory varchar(1024) not null,\n"
		"    primary key(id)\n"
		");\n"
		"create unique index imageservers_idx1\n"
		"    on imageservers(servername);\n"
	);
};

ImageServerRecord	ImageServerTableAdapter::row_to_object(int objectid,
				const Row& row) {
	ImageServerRecord	record(objectid);
	record.servername = row["servername"]->stringValue();
	record.database = row["dbname"]->stringValue();
	record.directory = row["directory"]->stringValue();
	return record;
}

UpdateSpec	ImageServerTableAdapter::object_to_updatespec(const ImageServerRecord& imageserver) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("servername", factory.get(imageserver.servername)));
	spec.insert(Field("dbname", factory.get(imageserver.database)));
	spec.insert(Field("directory", factory.get(imageserver.directory)));
	return spec;
}

ImageServer	ImageServerTable::get(const std::string& name) {
	std::string	condition = stringprintf("servername = '%s'",
				name.c_str());
	std::list<ImageServerRecord>	records = select(condition);
	if (0 == records.size()) {
		std::string	msg = stringprintf("no image server named '%s'",
			name.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	// since the server name is unique, we can be sure that we have
	// exactly one entry in the list at this point
	assert(records.size() == 1);
	ImageServerRecord	server = *records.begin();

	// convert this record to an ImageServer
	return ImageServer(DatabaseFactory::get(server.database),
		server.directory);
}

} // namespace project
} // namespace astro
