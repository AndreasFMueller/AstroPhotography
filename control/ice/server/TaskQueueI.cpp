/*
 * TaskQueueI.cpp -- task queue servant implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <TaskQueueI.h>
#include <algorithm>
#include <AstroFormat.h>
#include <sstream>
#include <ProxyCreator.h>
#include <IceConversions.h>

namespace snowstar {

TaskQueueI::TaskQueueI(astro::task::TaskQueue& _taskqueue)
	: taskqueue(_taskqueue) {
	// recover from crashes
	taskqueue.recover();

	// XXX install callback that publishes updates
}

TaskQueueI::~TaskQueueI() {
}

// interface methods
QueueState TaskQueueI::state(const Ice::Current& /* current */) {
	return convert(taskqueue.state());
}

void TaskQueueI::start(const Ice::Current& /* current */) {
	taskqueue.start();
}

void TaskQueueI::stop(const Ice::Current& /* current */) {
	taskqueue.stop();
}

int TaskQueueI::submit(const TaskParameters& parameters,
		const Ice::Current& /* current */) {
	return taskqueue.submit(snowstar::convert(parameters));
}

TaskParameters TaskQueueI::parameters(int taskid, const Ice::Current& /* current */) {
	return snowstar::convert(taskqueue.parameters(taskid));
}

TaskInfo TaskQueueI::info(int taskid, const Ice::Current& /* current */) {
	return snowstar::convert(taskqueue.info(taskid));
}

void TaskQueueI::cancel(int taskid, const Ice::Current& /* current */) {
	taskqueue.cancel(taskid);
}

void TaskQueueI::remove(int taskid, const Ice::Current& /* current */) {
	taskqueue.remove(taskid);
}

taskidsequence TaskQueueI::tasklist(TaskState state,
		const Ice::Current& /* current */) {
	std::list<long>	taskidlist = taskqueue.tasklist(snowstar::convert(state));
	taskidsequence	result;
	std::copy(taskidlist.begin(), taskidlist.end(), back_inserter(result));
	return result;
}

TaskPrx TaskQueueI::getTask(int taskid, const Ice::Current& current) {
	// make sure the task exists
	if (!taskqueue.exists(taskid)) {
		throw NotFound("task does not exist");
	}

	// create an identity for this task
	std::ostringstream	out;
	out << "task/" << taskid;
	std::string	identity = out.str();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "identity for task: %s",
		identity.c_str());

	// create the proxy
	return createProxy<TaskPrx>(identity, current);
}

} // namespace snowstar
