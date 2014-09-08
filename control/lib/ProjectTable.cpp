/*
 * ProjectTable.cpp -- implementation of the project table
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ProjectTable.h>
#include <includes.h>

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
		"    description varchar(1024) not null,\n"
		"    started datetime not null,\n"
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
	result.started = row["started"]->timeValue();
	return result;
}

UpdateSpec	ProjectTableAdapter::object_to_updatespec(const ProjectRecord& project) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("name", factory.get(project.name)));
	spec.insert(Field("description", factory.get(project.description)));
	spec.insert(Field("started", factory.getTime(project.started)));
	return spec;
}

} // namespace project
} // namespace astro
