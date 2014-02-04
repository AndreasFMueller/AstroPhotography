/*
 * TaskQueueI.cpp -- task queue servant implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <TaskQueueI.h>
#include <TypesI.h>
#include <CcdI.h>
#include <CcdIconversions.h>

namespace snowstar {

astro::task::TaskInfo::taskstate	convert(const TaskState& state) {
	switch (state) {
	case TskPENDING:
		return astro::task::TaskInfo::pending;
	case TskEXECUTING:
		return astro::task::TaskInfo::executing;
	case TskFAILED:
		return astro::task::TaskInfo::failed;
	case TskCANCELLED:
		return astro::task::TaskInfo::cancelled;
	case TskCOMPLETED:
		return astro::task::TaskInfo::complete;
	}
}

TaskState	convert(const astro::task::TaskInfo::taskstate& state) {
	switch (state) {
	case astro::task::TaskInfo::pending:
		return TskPENDING;
	case astro::task::TaskInfo::executing:
		return TskEXECUTING;
	case astro::task::TaskInfo::failed:
		return TskFAILED;
	case astro::task::TaskInfo::cancelled:
		return TskCANCELLED;
	case astro::task::TaskInfo::complete:
		return TskCOMPLETED;
	}
}

TaskInfo	convert(const astro::task::TaskInfo& info) {
	TaskInfo	result;
	result.taskid = info.id();
	result.state = convert(info.state());
	result.lastchange = info.lastchange();
	result.cause = info.cause();
	result.filename = info.filename();
	result.frame = snowstar::convert(info.frame());
	return result;
}

TaskParameters	convert(const astro::task::TaskParameters& parameters) {
	TaskParameters	result;
	result.camera = parameters.camera();
	result.ccdid = parameters.ccdid();
	result.ccdtemperature = parameters.ccdtemperature();
	result.filterwheel = parameters.filterwheel();
	result.filterposition = parameters.filterposition();
	result.exp = convert(parameters.exposure());
	return result;
}

astro::task::TaskParameters	convert(const TaskParameters& parameters) {
	astro::task::TaskParameters	result;
	result.exposure(convert(parameters.exp));
	result.camera(parameters.camera);
	result.ccdid(parameters.ccdid);
	result.ccdtemperature(parameters.ccdtemperature);
	result.filterwheel(parameters.filterwheel);
	result.filterposition(parameters.filterposition);
	return result;
}

QueueState	TaskQueueI::convert(const astro::task::TaskQueue::state_type& state) {
	switch (state) {
	case astro::task::TaskQueue::idle:
		return QueueIDLE;
	case astro::task::TaskQueue::launching:
		return QueueLAUNCHING;
	case astro::task::TaskQueue::stopping:
		return QueueSTOPPING;
	case astro::task::TaskQueue::stopped:
		return QueueSTOPPED;
	}
}

} // namespace snowstar
