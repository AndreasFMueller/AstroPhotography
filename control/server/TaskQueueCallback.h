/*
 * TaskQueueCallback.h -- callback for task queue updates
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _TaskQueueCallback_h
#define _TaskQueueCallback_h

#include <tasks.hh>
#include <TaskQueue_impl.h>
#include <AstroCallback.h>

namespace Astro {

class TaskQueueCallback : public astro::callback::Callback {
	TaskQueue_impl&	_taskqueue;
public:
	TaskQueueCallback(TaskQueue_impl& taskqueue);
	virtual astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data);
};

} // namespace astro

#endif /* _TaskQueueCallback_h */
