/*
 * TaskLocator.cpp -- locate a task servant implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <TaskLocator.h>
#include <TaskI.h>
#include <sstream>

namespace snowstar {

TaskLocator::TaskLocator(astro::persistence::Database& _database)
	: database(_database) {
}

Ice::ObjectPtr	TaskLocator::locate(const Ice::Current& current,
		Ice::LocalObjectPtr& cookie) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get a task %s",
		current.id.name.c_str());

	// parse the string
	std::istringstream	in(current.id.name);
	int	taskid;
	in >> taskid;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for task %d", taskid);

	// create the object
	Ice::ObjectPtr	object = new TaskI(database, taskid);

	return object;
}

void	TaskLocator::finished(const Ice::Current& current,
			const Ice::ObjectPtr& servant,
			const Ice::LocalObjectPtr& cookie) {
}

void	TaskLocator::deactivate(const std::string& category) {
}

} // namespace snowstar
