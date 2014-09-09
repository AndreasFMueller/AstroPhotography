/*
 * ProjectTable.h -- table of project records
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ProjectTable_h
#define _ProjectTable_h

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
	std::string	object;
	time_t	started;
	std::string	repository;
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
	ProjectRecord	get(const std::string& name);
	int	getid(const std::string& name);
	void	remove(const std::string& name);
};

} // namespace project
} // namespace astro

#endif /* _ProjectTable_h */
