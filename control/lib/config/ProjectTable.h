/*
 * ProjectTable.h -- table of project records
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ProjectTable_h
#define _ProjectTable_h

#include <AstroPersistence.h>
#include <AstroProject.h>

using namespace astro::persistence;

namespace astro {
namespace project {

//////////////////////////////////////////////////////////////////////
// Part object implementation
//////////////////////////////////////////////////////////////////////
/**
 * \brief Part information, as stored in the part table
 */
class PartInfo {
public:
	PartInfo() { }
	PartInfo(const Part& part);
	int	partno;
	std::string	instrument;
	int	width;
	int	height;
	int	xoffset;
	int	yoffset;
	double	exposuretime;
	double	gain;
	double	limit;
	int	binx;
	int	biny;
	int	shutter;
	int	purpose;
	int	quality;
	std::string	filtername;
	double	temperature;
	std::string	taskserver;
	int	taskid;
	int	repoid;
	operator	Part() const;
};

/**
 * \brief table adapter for the part table
 */
class PartRecord : public PersistentRef<PartInfo> {
public:
	PartRecord(int id, int projectid)
		: PersistentRef<PartInfo>(id, projectid) {
	}
	PartRecord(int id, int projectid, const Part& part);
	int	projectid() const { return ref(); }
};

/**
 * \brief Table adapter for the part table
 */
class PartTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static PartRecord	row_to_object(int objectid, const Row& row);
static UpdateSpec	object_to_updatespec(const PartRecord& part);
};

/**
 * \brief The part table
 */
class PartTable : public Table<PartRecord, PartTableAdapter> {
public:
	PartTable(Database database)
		: Table<PartRecord, PartTableAdapter>(database) {
	}
	using Table<PartRecord,PartTableAdapter>::id;
	virtual long	id(long projectid, long partno);
	using Table<PartRecord, PartTableAdapter>::add;
	long	add(long projectid, const Part& part);
	void	update(long projectid, const Part& part);
	bool	has(long projectid, long partno);
	Part	partById(long objectid);
	void	task(long projectid, long partno, long taskid);
	void	repo(long proejctid, long partno, long repoid);
};

//////////////////////////////////////////////////////////////////////
// Project database object implementation
//////////////////////////////////////////////////////////////////////
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
	operator	Project() const;
};

/**
 * \brief Persistent project information
 */
class ProjectRecord : public Persistent<ProjectInfo> {
public:
	ProjectRecord(int id = -1) : Persistent<ProjectInfo>(id) { }
	ProjectRecord(int id, const Project& project);
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
	PartTable	parttable;
public:
	ProjectTable(Database database)
		: Table<ProjectRecord, ProjectTableAdapter>(database),
		  parttable(database) {
	}
	ProjectRecord	get(const std::string& name);
	int	getid(const std::string& name);
	void	remove(const std::string& name);
	// interface to Project objects
	using Table<ProjectRecord, ProjectTableAdapter>::add;
	long	add(const Project& project);
	Project	projectById(long objectid);
	void	update(const Project& project);
};

} // namespace project
} // namespace astro

#endif /* _ProjectTable_h */
