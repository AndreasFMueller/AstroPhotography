/*
 * ImageReposTable.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "ImageReposTable.h"
#include "ImageRepoTables.h"
#include <AstroFormat.h>
#include <cassert>

namespace astro {
namespace project {

bool	ImageRepoInfo::operator==(const ImageRepoInfo& other) const {
	if (reponame != other.reponame) { return false; }
	if (database != other.database) { return false; }
	if (directory != other.directory) { return false; }
	if (hidden != other.hidden) { return false; }
	return true;
}

bool	ImageRepoRecord::operator==(const ImageRepoRecord& other) const {
	if (id() != other.id()) { return false; }
	if (reponame != other.reponame) { return false; }
	if (database != other.database) { return false; }
	if (directory != other.directory) { return false; }
	return true;
}

std::string	ImageRepoTableAdapter::tablename() {
	return std::string("imagerepos");
}

std::string	ImageRepoTableAdapter::createstatement() {
	return std::string(
		"create table imagerepos (\n"
		"    id int not null,\n"
		"    reponame varchar(32) not null,\n"
		"    dbname varchar(1024) not null,\n"
		"    directory varchar(1024) not null,\n"
		"    hidden int not null default 0,\n"
		"    primary key(id)\n"
		");\n"
		"create unique index imagerepos_idx1\n"
		"    on imagerepos(reponame);\n"
	);
};

ImageRepoRecord	ImageRepoTableAdapter::row_to_object(int objectid,
				const Row& row) {
	ImageRepoRecord	record(objectid);
	record.reponame = row["reponame"]->stringValue();
	record.database = row["dbname"]->stringValue();
	record.directory = row["directory"]->stringValue();
	record.hidden = row["hidden"]->intValue();
	return record;
}

UpdateSpec	ImageRepoTableAdapter::object_to_updatespec(const ImageRepoRecord& imagerepo) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("reponame", factory.get(imagerepo.reponame)));
	spec.insert(Field("dbname", factory.get(imagerepo.database)));
	spec.insert(Field("directory", factory.get(imagerepo.directory)));
	int	h = (imagerepo.hidden > 0) ? 1 : 0;
	spec.insert(Field("hidden", factory.get(h)));
	return spec;
}

bool	ImageRepoTable::contains(const std::string& name) {
	std::string	condition = stringprintf("reponame = '%s'",
				name.c_str());
	std::list<ImageRepoRecord>	records = select(condition);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d records for '%s'",
		records.size(), name.c_str());
	return (records.size() > 0);
}

ImageRepo	ImageRepoTable::get(const std::string& name) {
	std::string	condition = stringprintf("reponame = '%s'",
				name.c_str());
	std::list<ImageRepoRecord>	records = select(condition);
	if (0 == records.size()) {
		std::string	msg = stringprintf("no image repo named '%s'",
			name.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	// since the server name is unique, we can be sure that we have
	// exactly one entry in the list at this point
	assert(records.size() == 1);
	ImageRepoRecord	server = *records.begin();

	// convert this record to an ImageRepo
	return ImageRepo(name, DatabaseFactory::get(server.database),
		server.directory);
}

ImageRepoInfo	ImageRepoTable::getinfo(const std::string& name) {
	std::string	condition = stringprintf("reponame = '%s'",
				name.c_str());
	std::list<ImageRepoRecord>	records = select(condition);
	if (0 == records.size()) {
		std::string	msg = stringprintf("no image repo named '%s'",
			name.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	// since the server name is unique, we can be sure that we have
	// exactly one entry in the list at this point
	assert(records.size() == 1);
	ImageRepoRecord	reporecord = *records.begin();

	// convert this record to an ImageRepo
	ImageRepoInfo	info;
	info.id = reporecord.id();
	info.reponame = name;
	info.directory = reporecord.directory;
	info.database = reporecord.database;
	info.hidden = (reporecord.hidden > 0) ? true : false;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "info.hidden = %s",
		(info.hidden) ? "true" : "false");
	return info;
}

/**
 * \brief Remove a repo entry identified by name
 */
void	ImageRepoTable::remove(const std::string& name) {
	std::string	condition = stringprintf("reponame = '%s'",
				name.c_str());
	std::list<ImageRepoRecord>	records = select(condition);
	if (0 == records.size()) {
		std::string	msg = stringprintf("no image repo named '%s'",
			name.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	Table<ImageRepoRecord, ImageRepoTableAdapter>::remove(
		records.begin()->id());
}

} // namespace project
} // namespace astro
