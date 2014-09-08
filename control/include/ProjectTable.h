/*
 * ProjectTable.h -- table of project records
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroPersistence.h>

using namespace astro::persistence;

namespace astro {
namespace project {

/**
 * \brief Entries for the project table
 */
class ProjectInfo {
public:
	ProjectInfo();
	std::string	name;
	std::string	description;
	time_t	started;
};

/**
 * \brief Persistent project information
 */
class ProjectRecord : public Persistent<ProjectInfo> {
public:
	ProjectRecord(int id = -1) : Persistent<ProjectInfo>(id) { }
};

/**
 * \brief Adapter for the project table
 */
class ProjectTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static ProjectRecord	row_to_object(int objectid, const Row& row);
static UpdateSpec	object_to_updatespec(const ProjectRecord& project);
};

/** 
 * \brief The project table itself
 */
class ProjectTable : public Table<ProjectRecord, ProjectTableAdapter> {
public:
	ProjectTable(Database& database)
		: Table<ProjectRecord, ProjectTableAdapter>(database) {
	}
};

} // namespace project
} // namespace astro
