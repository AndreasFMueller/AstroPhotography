/*
 * ProjectConfiguration.cpp -- project configuration stuff
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConfig.h>
#include <AstroPersistence.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <cstdlib>
#include <AstroProject.h>
#include "ProjectTable.h"

using namespace astro::persistence;
using namespace astro::project;

namespace astro {
namespace config {

/**
 * \brief configuration backend
 *
 * This is used to hide the fact that there 
 */
class ProjectConfigurationBackend : public ProjectConfiguration {
	ConfigurationPtr	_config;
public:
	ProjectConfigurationBackend(ConfigurationPtr config) : _config(config) {
	}
	virtual ~ProjectConfigurationBackend() { }
	// project definition
	virtual project::Project	project(const std::string& name);
	virtual void	addproject(const project::Project& project);
	virtual void	removeproject(const std::string& name);
	virtual std::list<project::Project>	listprojects();

	// project part definition
	virtual project::PartPtr	part(const std::string& projectname,
						long partno);
	virtual void	addpart(const std::string& projectname,
				const project::Part& part);
	virtual void	removepart(const std::string& projectname, long partno);
	virtual std::list<project::PartPtr>	listparts(
					const std::string& projectname);
	virtual void	parttask(const std::string& projectname, long partno,
				int taskid);
	virtual void	partrepo(const std::string& projectname, long partno,
				int repoid);
};

//////////////////////////////////////////////////////////////////////
// static method implementation
//////////////////////////////////////////////////////////////////////
ProjectConfigurationPtr	ProjectConfiguration::get() {
	return ProjectConfigurationPtr(
			new ProjectConfigurationBackend(Configuration::get()));
}

ProjectConfigurationPtr	ProjectConfiguration::get(ConfigurationPtr config) {
	return ProjectConfigurationPtr(
			new ProjectConfigurationBackend(config));
}

//////////////////////////////////////////////////////////////////////
// project access
//////////////////////////////////////////////////////////////////////
/**
 * \brief Get a project from the configuration
 */
Project	ProjectConfigurationBackend::project(const std::string& name) {
	ProjectTable	projects(_config->database());
	long	projectid = projects.getid(name);
	return projects.projectById(projectid);
}

/**
 * \brief add a project to the configuration
 */
void	ProjectConfigurationBackend::addproject(const Project& project) {
	ProjectTable	projects(_config->database());
	projects.add(project);
}

/**
 * \brief Remove a project
 */
void	ProjectConfigurationBackend::removeproject(const std::string& name) {
	ProjectTable	projects(_config->database());
	projects.remove(name);
}

/**
 * \brief Get a list of projects defined in this configuration
 */
std::list<Project>	ProjectConfigurationBackend::listprojects() {
	std::list<Project>	result;
	ProjectTable	projects(_config->database());
	std::list<ProjectRecord>	records = projects.select("0 = 0");
	std::list<ProjectRecord>::const_iterator	pi;
	for (pi = records.begin(); pi != records.end(); pi++) {
		Project	project(pi->name);
		project.description(pi->description);
		project.object(pi->object);
		project.started(pi->started);
		project.repository(pi->repository);
		result.push_back(project);
	}
	return result;
}

//////////////////////////////////////////////////////////////////////
// Part access
//////////////////////////////////////////////////////////////////////

project::PartPtr        ProjectConfigurationBackend::part(const std::string& projectname,
				long partno) {
	return project(projectname).part(partno);
}

void    ProjectConfigurationBackend::addpart(const std::string& projectname, 
		const project::Part& part) {
	ProjectTable	projects(_config->database());
	int	projectid = projects.getid(projectname);
	PartTable	parts(_config->database());
	parts.add(projectid, part);
}

void    ProjectConfigurationBackend::removepart(const std::string& projectname,
		long partno) {
	std::string	query(	"delete from part "
				"where partno = ? and project = ("
				"  select id from projects where name = ?"
				")");
	StatementPtr	stmt = _config->database()->statement(query);
	FieldValueFactory	factory;
	stmt->bind(0, factory.get((int)partno));
	stmt->bind(1, factory.get(projectname));
	stmt->execute();
}

std::list<project::PartPtr>     ProjectConfigurationBackend::listparts(
					const std::string& projectname) {
	project::Project	proj = project(projectname);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found project, %d parts",
		proj.parts.size());
	std::list<project::PartPtr>	result;
	for (auto ptr = proj.parts.begin(); ptr != proj.parts.end(); ptr++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "part");
		result.push_back(ptr->second);
	}
	return result;
}

void	ProjectConfigurationBackend::parttask(const std::string& projectname,
		long partno, int taskid) {
	ProjectTable	projects(_config->database());
	int	projectid = projects.getid(projectname);
	PartTable	parts(_config->database());
	parts.task(projectid, partno, taskid);
}

void	ProjectConfigurationBackend::partrepo(const std::string& projectname,
		long partno, int repoid) {
	ProjectTable	projects(_config->database());
	int	projectid = projects.getid(projectname);
	PartTable	parts(_config->database());
	parts.repo(projectid, partno, repoid);
}

} // namespace config
} // namespace astro
