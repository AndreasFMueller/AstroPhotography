/*
 * ProjectTable.cpp -- implementation of the project table
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "ProjectTable.h"
#include <includes.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroPersistence.h>

namespace astro {
namespace project {

//////////////////////////////////////////////////////////////////////
// ProjectInfo implementation
//////////////////////////////////////////////////////////////////////
ProjectInfo::ProjectInfo() {
	started = time(NULL);
}

ProjectInfo::operator	Project() const {
	Project	project(name);
	project.description(description);
	project.object(object);
	project.repository(repository);
	project.started(started);
	return project;
}

//////////////////////////////////////////////////////////////////////
// ProjectRecord implementation
//////////////////////////////////////////////////////////////////////
ProjectRecord::ProjectRecord(int id, const Project& project)
	: Persistent<ProjectInfo>(id) {
	name = project.name();
	description = project.description();
	object = project.object();
	started = project.started();
	repository = project.repository();
}

//////////////////////////////////////////////////////////////////////
// ProjectTableAdapter implementation
//////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////
// ProjectTable implementation
//////////////////////////////////////////////////////////////////////
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

long	ProjectTable::add(const Project& project) {
	ProjectRecord	projectrecord(-1, project);
	long	projectid = add(projectrecord);
	std::map<long, PartPtr>::const_iterator	i;
	for (i = project.parts.begin(); i != project.parts.end(); i++) {
		PartRecord	partrecord(-1, projectid, *(i->second));
		parttable.add(partrecord);
	}
	return projectid;
}

Project	ProjectTable::projectById(long objectid) {
	Project	project = (Project)byid(objectid);
	std::string	condition = stringprintf("project = %d", objectid);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "select parts with condition '%s'",
		condition.c_str());
	std::list<PartRecord>	partlist = parttable.select(condition);
	std::list<PartRecord>::const_iterator	i;
	for (i = partlist.begin(); i != partlist.end(); i++) {
		PartPtr	part(new Part(*i));
		project.add(part);
	}
	return project;
}

void	ProjectTable::update(const Project& project) {
	long	projectid = getid(project.name());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "working on project %ld", projectid);
#if 0
	ProjectRecord	projectrecord(projectid, project);
	update(projectrecord);
	// XXX update all the parts too
#else
	throw std::runtime_error("incomplete implementation of ProjectTable");
#endif
}

//////////////////////////////////////////////////////////////////////
// PartRecord implementation
//////////////////////////////////////////////////////////////////////
PartRecord::PartRecord(int id, int projectid, const Part& part)
	: PersistentRef<PartInfo>(id, projectid, part) {
}

//////////////////////////////////////////////////////////////////////
// PartTableAdapter
//////////////////////////////////////////////////////////////////////
std::string	PartTableAdapter::tablename() {
	return std::string("part");
}

std::string	PartTableAdapter::createstatement() {
	return std::string(
		"create table part (\n"
		"    id integer not null,\n"
		"    project integer not null references projects(id) "
			"on delete cascade on update cascade,\n"
		"    partno integer not null,\n"
		"    instrument varchar(16) not null,\n"
		"    width integer not null,\n"
		"    height integer not null,\n"
		"    xoffset integer not null,\n"
		"    yoffset integer not null,\n"
		"    exposuretime float not null,\n"
		"    gain float not null,\n"
		"    vlimit float not null,\n"
		"    binx integer not null,\n"
		"    biny integer not null,\n"
		"    shutter integer not null,\n"
		"    purpose integer not null,\n"
		"    quality integer not null,\n"
		"    filtername varchar(32) not null,\n"
		"    temperature float not null,\n"
		"    taskserver varchar(64) not null,\n"
		"    task integer not null,\n"
		"    repoid integer not null,\n"
		"    primary key(id)\n"
		");\n"
		"create unique index part_idx1 "
			"on part(id, project);\n"
		"create unique index part_idx2 "
			"on part(project, partno);\n"
	);
}

PartRecord	PartTableAdapter::row_to_object(int objectid, const Row& row) {
	int	projectid = row["project"]->intValue();
	PartRecord	record(objectid, projectid);
	record.partno = row["partno"]->intValue();
	record.instrument = row["instrument"]->stringValue();
	record.width = row["width"]->intValue();
	record.height = row["height"]->intValue();
	record.xoffset = row["xoffset"]->intValue();
	record.yoffset = row["yoffset"]->intValue();
	record.exposuretime = row["exposuretime"]->doubleValue();
	record.gain = row["gain"]->doubleValue();
	record.limit = row["vlimit"]->doubleValue();
	record.binx = row["binx"]->intValue();
	record.biny = row["biny"]->intValue();
	record.shutter = row["shutter"]->intValue();
	record.purpose = row["purpose"]->intValue();
	record.quality = row["quality"]->intValue();
	record.filtername = row["filtername"]->stringValue();
	record.temperature = row["temperature"]->doubleValue();
	record.taskserver = row["taskserver"]->stringValue();
	record.taskid = row["task"]->intValue();
	record.repoid = row["repoid"]->intValue();
	return record;
}

UpdateSpec	PartTableAdapter::object_to_updatespec(const PartRecord& part) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("project", factory.get(part.projectid())));
	spec.insert(Field("partno", factory.get(part.partno)));
	spec.insert(Field("instrument", factory.get(part.instrument)));
	spec.insert(Field("width", factory.get(part.width)));
	spec.insert(Field("height", factory.get(part.height)));
	spec.insert(Field("xoffset", factory.get(part.xoffset)));
	spec.insert(Field("yoffset", factory.get(part.yoffset)));
	spec.insert(Field("exposuretime", factory.get(part.exposuretime)));
	spec.insert(Field("gain", factory.get(part.gain)));
	spec.insert(Field("vlimit", factory.get(part.limit)));
	spec.insert(Field("binx", factory.get(part.binx)));
	spec.insert(Field("biny", factory.get(part.biny)));
	spec.insert(Field("shutter", factory.get(part.shutter)));
	spec.insert(Field("purpose", factory.get(part.purpose)));
	spec.insert(Field("quality", factory.get(part.quality)));
	spec.insert(Field("filtername", factory.get(part.filtername)));
	spec.insert(Field("temperature", factory.get(part.temperature)));
	spec.insert(Field("taskserver", factory.get(part.taskserver)));
	spec.insert(Field("task", factory.get(part.taskid)));
	spec.insert(Field("repoid", factory.get(part.repoid)));
	return spec;
}

/**
 * \brief Cast operator to convert a PartInfo object to a part
 */
PartInfo::operator	Part() const {
	Part	part;
	part.partno(partno);
	part.instrument(instrument);
	astro::image::ImageSize	size(width, height);
	astro::image::ImagePoint	origin(xoffset, yoffset);
	astro::image::ImageRectangle	frame(size, origin);
	astro::camera::Exposure	exposure;
	exposure.frame(frame);
	exposure.exposuretime(exposuretime);
	exposure.gain(gain);
	exposure.limit(limit);
	exposure.mode(astro::image::Binning(binx, biny));
	exposure.shutter((shutter)	? astro::camera::Shutter::OPEN
					: astro::camera::Shutter::CLOSED);
	exposure.purpose((astro::camera::Exposure::purpose_t)purpose);
	exposure.quality((astro::camera::Exposure::quality_t)quality);
	part.exposure(exposure);
	part.filtername(filtername);
	part.temperature(temperature);
	part.taskserver(taskserver);
	part.taskid(taskid);
	part.repoid(repoid);
	return part;
}

/**
 * \brief Constructor to build Part from PartInfo
 */
PartInfo::PartInfo(const Part& part) {
	partno = part.partno();
	instrument = part.instrument();
	width = part.exposure().width();
	height = part.exposure().height();
	xoffset = part.exposure().x();
	yoffset = part.exposure().y();
	exposuretime = part.exposure().exposuretime();
	gain	 = part.exposure().gain();
	limit = part.exposure().limit();
	binx = part.exposure().mode().x();
	biny = part.exposure().mode().y();
	switch (part.exposure().shutter()) {
	case astro::camera::Shutter::OPEN:
		shutter = 1;
		break;
	case astro::camera::Shutter::CLOSED:
		shutter = 0;
		break;
	}
	purpose = part.exposure().purpose();
	quality = part.exposure().quality();
	filtername = part.filtername();
	temperature = part.temperature();
	taskserver = part.taskserver();
	taskid = part.taskid();
	repoid = part.repoid();
}

//////////////////////////////////////////////////////////////////////
// PartTable implementation
//////////////////////////////////////////////////////////////////////
long	PartTable::add(long projectid, const Part& part) {
	PartRecord	partrecord(-1, projectid, part);
	return add(partrecord);
}

long	PartTable::id(long projectid, long partno) {
	std::string	condition = stringprintf("id = %d and partno = %d",
		projectid, partno);
	return TableBase::id(condition);
}

void	PartTable::update(long projectid, const Part& part) {
	int	objectid = id(projectid, part.partno());
	PartRecord	partrecord(objectid, projectid, part);
	Table<PartRecord, PartTableAdapter>::update(objectid, partrecord);
}

bool	PartTable::has(long projectid, long partno) {
	std::string	condition = stringprintf("id = %d and partno = %d",
		projectid, partno);
	return	selectids(condition).size() > 0;
}

Part	PartTable::partById(long objectid) {
	return (Part)byid(objectid);
}

void	PartTable::task(long projectid, long partno, long taskid) {
	std::string	query(	"update part set task = ? "
				"where project = ? and partno = ?");
	StatementPtr	stmt = database()->statement(query);
	FieldValueFactory	factory;
	stmt->bind(0, factory.get((int)taskid));
	stmt->bind(1, factory.get((int)projectid));
	stmt->bind(2, factory.get((int)partno));
	stmt->execute();
}

void	PartTable::repo(long projectid, long partno, long repoid) {
	std::string	query(	"update part set repoid = ? "
				"where project = ? and partno = ?");
	StatementPtr	stmt = database()->statement(query);
	FieldValueFactory	factory;
	stmt->bind(0, factory.get((int)repoid));
	stmt->bind(1, factory.get((int)projectid));
	stmt->bind(2, factory.get((int)partno));
	stmt->execute();
}

} // namespace project
} // namespace astro
