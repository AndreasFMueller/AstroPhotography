/*
 * TaskQueue_impl.h -- servant for the Task queue
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _TaskQueue_impl_h
#define _TaskQueue_impl_h

#include <tasks.hh>
#include <AstroTask.h>
#include <MonitorChannel.h>

namespace Astro {

class TaskQueue_impl : public POA_Astro::TaskQueue {
	astro::task::TaskQueue&	_taskqueue;
public:
	TaskQueue_impl(astro::task::TaskQueue& taskqueue);

	virtual Astro::TaskQueue::QueueState	state();
	virtual ::CORBA::Long	submit(const ::Astro::TaskParameters& params);
	virtual TaskParameters	*parameters(::CORBA::Long taskid);
	virtual TaskInfo	*info(::CORBA::Long taskid);
	virtual void	cancel(::CORBA::Long taskid);
	virtual void	remove(::CORBA::Long taskid);
	virtual Astro::TaskQueue::taskidsequence	*tasklist(TaskState state);
	virtual void	start();
	virtual void	stop();
	virtual Task_ptr	getTask(::CORBA::Long taskid);
private:
	MonitorChannel< ::Astro::TaskMonitor, ::Astro::TaskMonitorInfo>
		monitorchannel;
public:
	virtual void	update(const Astro::TaskMonitorInfo& taskinf);
	virtual void	taskmonitor_stop();
	virtual ::CORBA::Long	registerMonitor(TaskMonitor_ptr taskmonitor);
	virtual void	unregisterMonitor(::CORBA::Long monitorid);
};

} // namespace Astro

#endif /* _TaskQueue_impl_h */
