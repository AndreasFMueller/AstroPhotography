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
#include <AstroDebug.h>

namespace snowstar {

template<>
void	callback_adapter<TaskMonitorPrx>(TaskMonitorPrx& p,
		const astro::callback::CallbackDataPtr data) {
	astro::task::TaskMonitorInfo	*taskmonitorinfo
		= dynamic_cast<astro::task::TaskMonitorInfo *>(&*data);

	// if there is no task monitor info, then give up immediately
	if (NULL == taskmonitorinfo) {
		return;
	}

	// send the information to the clients
	p->update(convert(*taskmonitorinfo));
}

TaskQueueI::TaskQueueI(astro::task::TaskQueue& _taskqueue)
	: taskqueue(_taskqueue) {
	// recover from crashes
	taskqueue.recover();

	// install callback that publishes updates
	TaskQueueICallback	*taskqueuecallback
		= new TaskQueueICallback(*this);
	taskqueue.callback = astro::callback::CallbackPtr(taskqueuecallback);
}

TaskQueueI::~TaskQueueI() {
}

// interface methods
QueueState TaskQueueI::state(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "state request");
	return convert(taskqueue.state());
}

void TaskQueueI::start(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start request");
	taskqueue.start();
}

void TaskQueueI::stop(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop request");
	taskqueue.stop();
}

int TaskQueueI::submit(const TaskParameters& parameters,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "submit a new task on '%s'",
		parameters.camera.c_str());
	return taskqueue.submit(snowstar::convert(parameters));
}

TaskParameters TaskQueueI::parameters(int taskid, const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "query parameters of task %d", taskid);
	return snowstar::convert(taskqueue.parameters(taskid));
}

TaskInfo TaskQueueI::info(int taskid, const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "query info of task %d", taskid);
	return snowstar::convert(taskqueue.info(taskid));
}

void TaskQueueI::cancel(int taskid, const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cancel request for %d", taskid);
	taskqueue.cancel(taskid);
}

void TaskQueueI::remove(int taskid, const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove request for %d", taskid);
	taskqueue.remove(taskid);
}

taskidsequence TaskQueueI::tasklist(TaskState state,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "list tasks in state %s",
		astro::task::TaskInfo::state2string(convert(state)).c_str());
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

void	TaskQueueI::registerMonitor(const Ice::Identity& callback,
		const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "register a new monitor callback");
	callbacks.registerCallback(callback, current);
}

void	TaskQueueI::unregisterMonitor(const Ice::Identity& callback,
		const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "unregistering a monitor callback");
	callbacks.unregisterCallback(callback, current);
}

void	TaskQueueI::taskUpdate(const astro::callback::CallbackDataPtr data) {
	callbacks(data);
}

} // namespace snowstar
