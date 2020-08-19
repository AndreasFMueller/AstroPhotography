/*
 * TaskQueueI.h -- Task queue servant declaration
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _TaskQueueI_h
#define _TaskQueueI_h

#include <tasks.h>
#include <AstroTask.h>
#include <CallbackHandler.h>
#include <AstroDebug.h>
#include "StatisticsI.h"

namespace snowstar {

template<>
void	callback_adapter<TaskMonitorPrx>(TaskMonitorPrx p,
		const astro::callback::CallbackDataPtr d);

class TaskQueueI : virtual public TaskQueue, public StatisticsI {
	astro::task::TaskQueue&	taskqueue;
private:
	TaskQueueI(const TaskQueueI& other);
	TaskQueue&	operator=(const TaskQueueI& other);
public:
	TaskQueueI(astro::task::TaskQueue& taskqueue);
	virtual	~TaskQueueI();
	// conversion 

	// interface methods
	virtual QueueState state(const Ice::Current& current);
	virtual void start(const Ice::Current& current);
	virtual void stop(const Ice::Current& current);
	virtual int submit(const TaskParameters&,
			const Ice::Current& current);
	virtual TaskParameters parameters(int taskid,
			const Ice::Current& current);
	virtual TaskInfo info(int taskid, const Ice::Current& current);
	virtual void cancel(int taskid, const Ice::Current& current);
	virtual void remove(int taskid, const Ice::Current& current);
	virtual void resubmit(int taskid, const Ice::Current& current);
	virtual taskidsequence tasklist(TaskState state,
			const Ice::Current& current);
	virtual TaskPrx getTask(int taskid, const Ice::Current& current);

	// callback handlers
private:
	SnowCallback<TaskMonitorPrx>	callbacks;
public:
	virtual void	registerMonitor(const Ice::Identity& callback,
				const Ice::Current& current);
	virtual void	unregisterMonitor(const Ice::Identity& callbac,
				const Ice::Current& current);
	void	taskUpdate(const astro::callback::CallbackDataPtr data);
};

/**
 * \brief Callback class for task monitoring
 */
class TaskQueueICallback : public astro::callback::Callback {
	TaskQueueI&	_taskqueue;
public:
	TaskQueueICallback(TaskQueueI& taskqueue) : _taskqueue(taskqueue) { }
	virtual astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "TaskQueueICallback::operator()");
		_taskqueue.taskUpdate(data);
		return data;
	}
};

} // namespace snowstar

#endif /* _TaskQueueI_h */
