/*
 * TaskConversions.cpp -- conversions between ice and astro
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <IceConversions.h>
#include <includes.h>
#include <AstroIO.h>
#include <AstroUtils.h>

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
	case TskCOMPLETE:
		return astro::task::TaskInfo::complete;
	}
	throw std::runtime_error("unknown task state");
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
		return TskCOMPLETE;
	}
	throw std::runtime_error("unknown task state");
}

std::string	state2string(TaskState s) {
	return astro::task::TaskInfo::state2string(convert(s));
}

TaskState	string2taskstate(const std::string& s) {
	return convert(astro::task::TaskInfo::string2state(s));
}

TaskInfo	convert(const astro::task::TaskInfo& info) {
	TaskInfo	result;
	result.taskid = info.id();
	result.state = convert(info.state());
	result.lastchange = converttime((time_t)info.lastchange());
	result.cause = info.cause();
	result.filename = info.filename();
	result.frame = snowstar::convert(info.frame());
	return result;
}

astro::task::TaskInfo	convert(const TaskInfo& info) {
	astro::task::TaskInfo	result(info.taskid);
	result.state(convert(info.state));
	result.lastchange(converttime(info.lastchange));
	result.cause(info.cause);
	result.filename(info.filename);
	result.frame(snowstar::convert(info.frame));
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

QueueState	convert(const astro::task::TaskQueue::state_type& state) {
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
	throw std::runtime_error("unknown queue state");
}

astro::task::TaskQueue::state_type	convert(const QueueState& state) {
	switch (state) {
	case QueueIDLE:
		return astro::task::TaskQueue::idle;
	case QueueLAUNCHING:
		return astro::task::TaskQueue::launching;
	case QueueSTOPPING:
		return astro::task::TaskQueue::stopping;
	case QueueSTOPPED:
		return astro::task::TaskQueue::stopped;
	}
	throw std::runtime_error("unknown queue state");
}

std::string	state2string(QueueState s) {
	return astro::task::TaskQueue::state2string(convert(s));
}

QueueState	string2queuestate(const std::string& s) {
	return convert(astro::task::TaskQueue::string2state(s));
}

TaskMonitorInfo convert(const astro::task::TaskMonitorInfo& monitorinfo) {
	TaskMonitorInfo	result;
	result.taskid = monitorinfo.taskid();
	result.newstate = convert(monitorinfo.state());
	result.timeago = converttime(monitorinfo.when());
	return result;
}

astro::task::TaskMonitorInfo    convert(const TaskMonitorInfo& monitorinfo) {
	astro::task::TaskMonitorInfo	result;
	result.taskid(monitorinfo.taskid);
	result.state(convert(monitorinfo.newstate));
	result.when(converttime(monitorinfo.timeago));
	return result;
}

} // namespace snowstar
