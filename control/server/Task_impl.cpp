/*
 * Task_impl.cpp -- CORBA Task servent implementation
 * 
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Task_impl.h"
#include "Conversions.h"
#include <OrbSingleton.h>

namespace Astro {

/**
 * \brief Construct an Image servant from a file
 */
Task_impl::Task_impl(astro::task::TaskTable& tasktable, long queueid)
	: _tasktable(tasktable), _queueid(queueid) {
}

astro::task::TaskQueueEntry	Task_impl::entry() {
	if (!_tasktable.exists(_queueid)) {
		throw CORBA::OBJECT_NOT_EXIST();
	}
	astro::task::TaskQueueEntry	result = _tasktable.byid(_queueid);
	return result;
}

Astro::TaskState	Task_impl::state() {
	return astro::convert(entry().state());
}

Astro::TaskInfo	*Task_impl::info() {
	TaskInfo	*info = new TaskInfo();
	astro::task::TaskInfo	taskinfo = entry().info();
	(*info) = astro::convert(taskinfo);
	return info;
}

Astro::TaskParameters	*Task_impl::parameters() {
	TaskParameters	*params = new TaskParameters();
	astro::task::TaskParameters	taskparameters = entry().parameters();
	(*params) = astro::convert(taskparameters);
	return params;
}

Image_ptr	Task_impl::getImage() {
}

char	*Task_impl::imagename() {
	return CORBA::string_dup(entry().filename().c_str());
}

} // namespace Astro
