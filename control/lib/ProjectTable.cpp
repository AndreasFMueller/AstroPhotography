/*
 * ProjectTable.cpp -- implementation of the project table
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ProjectTable.h>
#include <includes.h>
#include <AstroDebug.h>
#include <AstroFormat.h>

namespace astro {
namespace project {

ProjectInfo::ProjectInfo() {
	started = time(NULL);
}

std::string	ProjectTableAdapter::tablename() {
	return std::string("projects");
}

std::string	ProjectTableAdapter::createstatement() {
	return std::string(
		"create table projects (\n"
		"    id int not null,\n"
		"    name varchar(32) not null,\n"
		"    description varchar(1024) not null default '',\n"
		"    object varchar(256) not null default '',\n"
		"    started datetime not null,\n"
		"    repository varchar(8) not null,\n"
		"    primary key(id)\n"
		");\n"
		"create unique index projects_idx1 on projects(name);\n"
	);
}

ProjectRecord	ProjectTableAdapter::row_to_object(int objectid,
			const Row& row) {
	ProjectRecord	result(objectid);
	result.name = row["name"]->stringValue();
	result.description = row["description"]->stringValue();
	result.object = row["object"]->stringValue();
	result.started = row["started"]->timeValue();
	result.repository = row["repository"]->stringValue();
	return result;
}

UpdateSpec	ProjectTableAdapter::object_to_updatespec(const ProjectRecord& project) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("name", factory.get(project.name)));
	spec.insert(Field("description", factory.get(project.description)));
	spec.insert(Field("object", factory.get(project.object)));
	spec.insert(Field("started", factory.getTime(project.started)));
	spec.insert(Field("repository", factory.get(project.repository)));
	return spec;
}

ProjectRecord	ProjectTable::get(const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve project '%s'", name.c_str());
	std::string	condition = stringprintf("name = '%s'",
		database()->escape(name).c_str());
	std::list<ProjectRecord>	l = select(condition);
	if (0 == l.size()) {
		std::string	msg = stringprintf("no project '%s'",
			name.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return *(l.begin());
}

int	ProjectTable::getid(const std::string& name) {
	return get(name).id();
}

void	ProjectTable::remove(const std::string& name) {
	Table<ProjectRecord, ProjectTableAdapter>::remove(getid(name));
}

} // namespace project
} // namespace astro
