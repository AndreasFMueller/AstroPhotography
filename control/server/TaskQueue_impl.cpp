/*
 * TaskQueue_impl.cpp -- task queue servant implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "TaskQueue_impl.h"
#include <AstroDebug.h>
#include <Conversions.h>
#include <AstroFormat.h>
#include <OrbSingleton.h>

namespace Astro {

TaskQueue_impl::TaskQueue_impl(astro::task::TaskQueue& taskqueue)
	: _taskqueue(taskqueue) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "task queue servant created");
}

/**
 * \brief get the state of the task queue
 */
Astro::TaskQueue::QueueState	TaskQueue_impl::state() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "query queue state");
	return astro::convert(_taskqueue.state());
}

/**
 * \brief start the queue
 */
void	TaskQueue_impl::start() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start the queue");
	_taskqueue.start();
}

/**
 * \brief stop the queue
 */
void	TaskQueue_impl::stop() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop the queue");
	_taskqueue.stop();
}

/**
 * \brief Submit a new task to the queue
 */
CORBA::Long	TaskQueue_impl::submit(const TaskParameters& params) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "submit new task");

	astro::task::TaskParameters	parameters = astro::convert(params);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure time: %f",
		parameters.exposure().exposuretime);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "origin: (%d,%d)",
		parameters.exposure().frame.origin().x(),
		parameters.exposure().frame.origin().y());

	// submit the task to the 
	return _taskqueue.submit(parameters);
}

/**
 * \brief Retrieve the parameters of a task
 */
TaskParameters	*TaskQueue_impl::parameters(CORBA::Long taskid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve parameters of task %ld",
		taskid);
	try {
		// get the task parameters
		astro::task::TaskExecutorPtr	executor
			= _taskqueue.executor(taskid);
		astro::task::TaskQueueEntry	entry = executor->task();

		// allocate a parameter struct
		TaskParameters	*parameters = new TaskParameters();

		// convert the parameters
		*parameters = astro::convert(entry.parameters());

		return parameters;
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", x.what());
		throw BadParameter(x.what());
	}
}

/**
 * \brief Retrieve the parameters of a task
 */
TaskInfo	*TaskQueue_impl::info(CORBA::Long taskid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve info of task %ld", taskid);
	try {
		// get the task parameters
		astro::task::TaskExecutorPtr	executor
			= _taskqueue.executor(taskid);
		astro::task::TaskQueueEntry	entry = executor->task();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image frame: %s",
			entry.frame().toString().c_str());

		// allocate a parameter struct
		TaskInfo	*info = new TaskInfo();

		// convert the parameters
		*info = astro::convert(entry.info());

		return info;
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", x.what());
		throw BadParameter(x.what());
	}
}

/**
 * \brief Cancel 
 */
void	TaskQueue_impl::cancel(CORBA::Long taskid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cancel task %ld", taskid);
	try {
		//_taskqueue.cancel(taskid);
	} catch (const std::exception& x) {
		
	}
}

/**
 * \brief Remove a task from the queue
 */
void	TaskQueue_impl::remove(CORBA::Long taskid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove task %ld", taskid);
}

/**
 * \brief retrieve a list of identifiers
 */
Astro::TaskQueue::taskidsequence	*TaskQueue_impl::tasklist(TaskState state) {
	std::list<long>	taskidlist = _taskqueue.tasklist(astro::convert(state));

	Astro::TaskQueue::taskidsequence	*result
		= new Astro::TaskQueue::taskidsequence();
        result->length(taskidlist.size());
	std::list<long>::const_iterator	i;
	int	j = 0;
	for (i = taskidlist.begin(); i != taskidlist.end(); i++) {
		(*result)[j++] = *i;
	}
        return result;

}

/**
 * \brief Get an reference to a task
 */
Task_ptr	TaskQueue_impl::getTask(::CORBA::Long taskid) {
	if (!_taskqueue.exists(taskid)) {
		throw CORBA::OBJECT_NOT_EXIST();
	}

	// create a reference from the id
	std::string	stringid = astro::stringprintf("%08d", taskid);

        // create an object id associated with the file name
        PortableServer::ObjectId_var    oid
                = PortableServer::string_to_ObjectId(stringid.c_str());
        debug(LOG_DEBUG, DEBUG_LOG, 0, "oid %s created", stringid.c_str());

        // now create an object reference in the POA for images
	OrbSingleton    orb;
	PoaName poapath("Tasks");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting POA for Tasks");
	PortableServer::POA_var tasks_poa = orb.findPOA(poapath);
	CORBA::Object_var	obj
		= tasks_poa->create_reference_with_id(oid,
			"IDL:/Astro/Task");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reference for task created");
	return Task::_narrow(obj);
}

/**
 * \brief register a Task monitor
 */
CORBA::Long	TaskQueue_impl::registerMonitor(TaskMonitor_ptr taskmonitor) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "register a monitor");
	return monitorchannel.subscribe(taskmonitor);
}

/**
 * \brief unregister a task monitor 
 */
void	TaskQueue_impl::unregisterMonitor(::CORBA::Long monitorid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "unregister the monitor %d", monitorid);
	monitorchannel.unsubscribe(monitorid);
}

/**
 * \brief update all monitors
 */
void	TaskQueue_impl::update(const ::Astro::TaskMonitorInfo& taskinfo) {
	monitorchannel.update(taskinfo);
}

/**
 * \brief Distribute the stop signal to all monitors
 */
void	TaskQueue_impl::taskmonitor_stop() {
	monitorchannel.stop();
}

} // namespace Astro
