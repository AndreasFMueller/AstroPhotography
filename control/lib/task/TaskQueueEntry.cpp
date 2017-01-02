/*
 * TaskQueueEntry.cpp -- Implementation of the TaskQueueEntry class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTask.h>
#include <AstroDebug.h>

namespace astro {
namespace task {

TaskQueueEntry::TaskQueueEntry(taskid_t queueid, const TaskParameters& task)
	: TaskParameters(task), TaskInfo(queueid) {
	state(pending);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filterwheel: %s",
		filterwheel().c_str());
}

TaskParameters	TaskQueueEntry::parameters() const {
	TaskParameters	parameters;
	parameters.exposure(exposure());
	parameters.cameraindex(cameraindex());
	parameters.ccdindex(ccdindex());
	parameters.coolerindex(coolerindex());
	parameters.ccdtemperature(ccdtemperature());
	parameters.filterwheelindex(filterwheelindex());
	parameters.filter(filter());
	parameters.instrument(instrument());
	parameters.mountindex(mountindex());
	parameters.project(project());
	parameters.repository(repository());
	return parameters;
}

TaskInfo	TaskQueueEntry::info() const {
	TaskInfo	info(id());
	info.state(state());
	info.lastchange(lastchange());
	info.cause(cause());
	info.filename(filename());
	info.frame(frame());
	info.camera(camera());
	info.ccd(ccd());
	info.cooler(cooler());
	info.filterwheel(filterwheel());
	info.mount(mount());
	return info;
}

bool	TaskQueueEntry::blocks(const TaskQueueEntry& other) const {
	// only if the other state is pending it can be blocked
	if (pending != other.state()) {
		return false;
	}

	// if this task is failed or cancelled, it cannot block anything
	// either
	if ((failed == state()) || (cancelled == state())) {
		return false;
	}

	// This task blocks some other task if there is some resource
	// that both use, e.g. if both use the same camera and ccd
	if ((camera() == other.camera()) || (ccd() == other.ccd())) {
		return true;
	}

	// if we use the same coolers, we also block
	if (0 != cooler().size()) {
		if (cooler() == other.cooler()) {
			return true;
		}
	}

	// we also have a problem if a filter wheel is in use
	if (0 != filterwheel().size()) {
		if (filterwheel() == other.filterwheel()) {
			return true;
		}
	}

	// If none of the conditions above happens 
	return false;
}

/**
 * \brief check whether this task is blocked by some other task
 */
bool	TaskQueueEntry::blockedby(const TaskQueueEntry& other) const {
	return other.blocks(*this);
}

} // namespace task
} // namespace astro
