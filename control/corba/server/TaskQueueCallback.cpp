/*
 * TaskQueueCallback.cpp -- taskqueue callback implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <TaskQueueCallback.h>
#include <Conversions.h>

namespace Astro {

/**
 * \brief Create a callback object
 */
TaskQueueCallback::TaskQueueCallback(TaskQueue_impl& taskqueue)
	: _taskqueue(taskqueue) {
}

using astro::callback::CallbackDataPtr;
using astro::task::TaskMonitorCallbackData;

/**
 * \brief perform the callback
 */
CallbackDataPtr	TaskQueueCallback::operator()(CallbackDataPtr data) {
	// extract the information from the data from the argument
	TaskMonitorCallbackData	*tmcd
		= dynamic_cast<TaskMonitorCallbackData *>(&*data);
	if (NULL == tmcd) {
		// no data of the expected type, so we just ignore it
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"callback called with useless argument");
		return data;
	}

	// send the udpate to the taskqueue
	_taskqueue.update(astro::convert(tmcd->data()));

	// that's it
	return data;
}


} // namespace Astro
