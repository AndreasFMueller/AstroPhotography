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

astro::task::tasktype	convert(const TaskType& type) {
	switch (type) {
	case TaskEXPOSURE:
		return astro::task::tasktype::EXPOSURE;
	case TaskDITHER:
		return astro::task::tasktype::DITHER;
	case TaskFOCUS:
		return astro::task::tasktype::FOCUS;
	case TaskSLEEP:
		return astro::task::tasktype::SLEEP;
	}
	throw std::runtime_error("unknown task type");
}

TaskType	convert(const astro::task::tasktype& type) {
	int	t = type;
	switch (t) {
	case astro::task::tasktype::EXPOSURE:
		return TaskEXPOSURE;
	case astro::task::tasktype::DITHER:
		return TaskDITHER;
	case astro::task::tasktype::FOCUS:
		return TaskFOCUS;
	case astro::task::tasktype::SLEEP:
		return TaskSLEEP;
	}
	throw std::runtime_error("unknown task type");
}

std::string	tasktype2string(const TaskType& type) {
	switch (type) {
	case TaskEXPOSURE:
		return std::string("exposure");
	case TaskDITHER:
		return std::string("dither");
	case TaskFOCUS:
		return std::string("focus");
	case TaskSLEEP:
		return std::string("sleep");
	}
	throw std::runtime_error("unknown task type");
}

TaskType	string2tasktype(const std::string& type) {
	if (type == std::string("exposure")) {
		return TaskEXPOSURE;
	}
	if (type == std::string("dither")) {
		return TaskDITHER;
	}
	if (type == std::string("focus")) {
		return TaskFOCUS;
	}
	if (type == std::string("sleep")) {
		return TaskSLEEP;
	}
	throw std::runtime_error("unknown task type");
}

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
	case TskDELETED:
		return astro::task::TaskInfo::deleted;
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
	case astro::task::TaskInfo::deleted:
		return TskDELETED;
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
	result.camera = info.camera();
	result.ccd = info.ccd();
	result.cooler = info.cooler();
	result.filterwheel = info.filterwheel();
	result.mount = info.mount();
	result.focuser = info.focuser();
	result.guiderccd = info.guiderccd();
	result.guideport = info.guideport();
	result.adaptiveoptics = info.adaptiveoptics();
	return result;
}

astro::task::TaskInfo	convert(const TaskInfo& info) {
	astro::task::TaskInfo	result(info.taskid);
	result.state(convert(info.state));
	result.lastchange(converttime(info.lastchange));
	result.cause(info.cause);
	result.filename(info.filename);
	result.frame(snowstar::convert(info.frame));
	result.camera(info.camera);
	result.ccd(info.ccd);
	result.cooler(info.cooler);
	result.filterwheel(info.filterwheel);
	result.mount(info.mount);
	result.focuser(info.focuser);
	result.guiderccd(info.guiderccd);
	result.guideport(info.guideport);
	result.adaptiveoptics(info.adaptiveoptics);
	return result;
}

TaskParameters	convert(const astro::task::TaskParameters& parameters) {
	TaskParameters	result;
	result.type = convert(parameters.taskType());
	result.instrument = parameters.instrument();
	result.cameraIndex = parameters.cameraindex();
	result.ccdIndex = parameters.ccdindex();
	result.coolerIndex = parameters.coolerindex();
	result.ccdtemperature = parameters.ccdtemperature();
	result.filterwheelIndex = parameters.filterwheelindex();
	result.filter = parameters.filter();
	result.mountIndex = parameters.mountindex();
	result.focuserIndex = parameters.focuserindex();
	result.guiderccdIndex = parameters.guiderccdindex();
	result.guideportIndex = parameters.guideportindex();
	result.adaptiveopticsIndex = parameters.adaptiveopticsindex();
	result.project = parameters.project();
	result.repodb = parameters.repodb();
	result.repository = parameters.repository();
	result.exp = convert(parameters.exposure());
	return result;
}

astro::task::TaskParameters	convert(const TaskParameters& parameters) {
	astro::task::TaskParameters	result;
	result.taskType(convert(parameters.type));
	result.exposure(convert(parameters.exp));
	result.instrument(parameters.instrument);
	result.cameraindex(parameters.cameraIndex);
	result.ccdindex(parameters.ccdIndex);
	result.coolerindex(parameters.coolerIndex);
	result.ccdtemperature(parameters.ccdtemperature);
	result.filterwheelindex(parameters.filterwheelIndex);
	result.filter(parameters.filter);
	result.mountindex(parameters.mountIndex);
	result.focuserindex(parameters.focuserIndex);
	result.guiderccdindex(parameters.guiderccdIndex);
	result.guideportindex(parameters.guideportIndex);
	result.adaptiveopticsindex(parameters.adaptiveopticsIndex);
	result.project(parameters.project);
	result.repodb(parameters.repodb);
	result.repository(parameters.repository);
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
	result.type = convert(monitorinfo.taskType());
	result.timeago = converttime(monitorinfo.when());
	return result;
}

astro::task::TaskMonitorInfo    convert(const TaskMonitorInfo& monitorinfo) {
	astro::task::TaskMonitorInfo	result;
	result.taskid(monitorinfo.taskid);
	result.taskType(astro::task::tasktype(monitorinfo.type));
	result.state(convert(monitorinfo.newstate));
	result.when(converttime(monitorinfo.timeago));
	return result;
}

astro::gateway::TaskUpdate convert(const StatusUpdate& statusupdate) {
	astro::gateway::TaskUpdate	result(statusupdate.instrument);
	result.updatetime = converttime(statusupdate.updatetimeago);
        result.avgguideerror = statusupdate.avgguideerror;
        result.ccdtemperature = statusupdate.ccdtemperature + 273.15;
        result.lastimagestart = converttime(statusupdate.lastimagestartago);
        result.exposuretime = statusupdate.exposuretime;
        result.currenttaskid = statusupdate.currenttaskid;
        result.telescope = astro::RaDec(
		astro::Angle(statusupdate.telescope.ra,
			astro::Angle::Hours),
		astro::Angle(statusupdate.telescope.dec,
			astro::Angle::Degrees));
        result.west = statusupdate.west;
        result.filter = statusupdate.filter;
	result.observatory = astro::LongLat(
		astro::Angle(statusupdate.observatory.longitude,
			astro::Angle::Degrees),
		astro::Angle(statusupdate.observatory.latitude,
			astro::Angle::Degrees));
	result.project = statusupdate.project;
	result.focus = statusupdate.focus;
	return result;
}

StatusUpdate    convert(const astro::gateway::TaskUpdate& taskupdate) {
	StatusUpdate	result;
	result.instrument = taskupdate.instrument();
	result.updatetimeago = converttime(taskupdate.updatetime);
	result.avgguideerror = taskupdate.avgguideerror;
	result.ccdtemperature = taskupdate.ccdtemperature - 273.15;
	result.lastimagestartago = converttime(taskupdate.lastimagestart);
	result.exposuretime = taskupdate.exposuretime;
	result.currenttaskid = taskupdate.currenttaskid;
	result.telescope.ra = taskupdate.telescope.ra().hours();
	result.telescope.dec = taskupdate.telescope.dec().degrees();
	result.west = taskupdate.west;
	result.filter = taskupdate.filter;
	result.observatory.longitude
		= taskupdate.observatory.longitude().degrees();
	result.observatory.latitude
		= taskupdate.observatory.latitude().degrees();
	result.project = taskupdate.project;
	result.focus = taskupdate.focus;
	return result;
}

} // namespace snowstar
