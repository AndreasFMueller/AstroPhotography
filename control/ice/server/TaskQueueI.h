/*
 * TaskQueueI.h -- Task queue servant declaration
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _TaskQueueI_h
#define _TaskQueueI_h

#include <tasks.h>
#include <AstroTask.h>

namespace snowstar {

TaskState	convert(const astro::task::TaskInfo::taskstate& state);
astro::task::TaskInfo::taskstate	convert(const TaskState& state);

TaskInfo	convert(const astro::task::TaskInfo& info);

TaskParameters	convert(const astro::task::TaskParameters& parameters);
astro::task::TaskParameters	convert(const TaskParameters& parameters);

class TaskQueueI : public TaskQueue {
	astro::task::TaskQueue&	taskqueue;
public:
	TaskQueueI(astro::task::TaskQueue& taskqueue);
	virtual	~TaskQueueI();
	// conversion 
	QueueState	convert(const astro::task::TaskQueue::state_type& state);

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
	virtual taskidsequence tasklist(TaskState state,
			const Ice::Current& current);
	virtual TaskPrx getTask(int taskid, const Ice::Current& current);
};

} // namespace snowstar

#endif /* _TaskQueueI_h */
