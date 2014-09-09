/*
 * Conversions.cpp -- type conversions
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Conversions.h>
#include <AstroDebug.h>
#include <stdexcept>
#include <AstroImage.h>
#include <AstroCamera.h>
#include <AstroUtils.h>

namespace astro {

// Device type

Astro::DeviceLocator::device_type	convert(
	astro::DeviceName::device_type fromtype) {
	switch (fromtype) {
	case astro::DeviceName::AdaptiveOptics:
		return Astro::DeviceLocator::DEVICE_AO;
	case astro::DeviceName::Camera:
		return Astro::DeviceLocator::DEVICE_CAMERA;
	case astro::DeviceName::Ccd:
		return Astro::DeviceLocator::DEVICE_CCD;
	case astro::DeviceName::Cooler:
		return Astro::DeviceLocator::DEVICE_COOLER;
	case astro::DeviceName::Filterwheel:
		return Astro::DeviceLocator::DEVICE_FILTERWHEEL;
	case astro::DeviceName::Focuser:
		return Astro::DeviceLocator::DEVICE_FOCUSER;
	case astro::DeviceName::Guiderport:
		return Astro::DeviceLocator::DEVICE_GUIDERPORT;
	case astro::DeviceName::Module:
		return Astro::DeviceLocator::DEVICE_MODULE;
	case astro::DeviceName::Mount:
		return Astro::DeviceLocator::DEVICE_MOUNT;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "illegal type: %d", fromtype);
	throw std::runtime_error("illegal type");
}

astro::DeviceName::device_type	convert(
		Astro::DeviceLocator::device_type fromtype) {
	switch (fromtype) {
	case Astro::DeviceLocator::DEVICE_AO:
		return astro::DeviceName::AdaptiveOptics;
	case Astro::DeviceLocator::DEVICE_CAMERA:
		return astro::DeviceName::Camera;
	case Astro::DeviceLocator::DEVICE_CCD:
		return astro::DeviceName::Ccd;
	case Astro::DeviceLocator::DEVICE_COOLER:
		return astro::DeviceName::Cooler;
	case Astro::DeviceLocator::DEVICE_FILTERWHEEL:
		return astro::DeviceName::Filterwheel;
	case Astro::DeviceLocator::DEVICE_FOCUSER:
		return astro::DeviceName::Focuser;
	case Astro::DeviceLocator::DEVICE_GUIDERPORT:
		return astro::DeviceName::Guiderport;
	case Astro::DeviceLocator::DEVICE_MODULE:
		return astro::DeviceName::Module;
	case Astro::DeviceLocator::DEVICE_MOUNT:
		return astro::DeviceName::Mount;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "illegal type: %d", fromtype);
	throw std::runtime_error("illegal type");
}

std::string	convert2string(astro::DeviceName::device_type fromtype) {
	switch (fromtype) {
	case astro::DeviceName::AdaptiveOptics:
		return std::string("AO");
	case astro::DeviceName::Camera:
		return std::string("CAMERA");
	case astro::DeviceName::Ccd:
		return std::string("CCD");
	case astro::DeviceName::Cooler:
		return std::string("COOLER");
	case astro::DeviceName::Filterwheel:
		return std::string("FILTERWHEEL");
	case astro::DeviceName::Focuser:
		return std::string("FOCUSER");
	case astro::DeviceName::Guiderport:
		return std::string("GUIDERPORT");
	case astro::DeviceName::Module:
		return std::string("MODULE");
	case astro::DeviceName::Mount:
		return std::string("MOUNT");
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "illegal type: %d", fromtype);
	throw std::runtime_error("illegal type");
}

std::string	convert2string(Astro::DeviceLocator::device_type fromtype) {
	switch (fromtype) {
	case Astro::DeviceLocator::DEVICE_AO:
		return std::string("AO");
	case Astro::DeviceLocator::DEVICE_CAMERA:
		return std::string("CAMERA");
	case Astro::DeviceLocator::DEVICE_CCD:
		return std::string("CCD");
	case Astro::DeviceLocator::DEVICE_COOLER:
		return std::string("COOLER");
	case Astro::DeviceLocator::DEVICE_FILTERWHEEL:
		return std::string("FILTERWHEEL");
	case Astro::DeviceLocator::DEVICE_FOCUSER:
		return std::string("FOCUSER");
	case Astro::DeviceLocator::DEVICE_GUIDERPORT:
		return std::string("GUIDERPORT");
	case Astro::DeviceLocator::DEVICE_MODULE:
		return std::string("MODULE");
	case Astro::DeviceLocator::DEVICE_MOUNT:
		return std::string("MOUNT");
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "illegal type: %d", fromtype);
	throw std::runtime_error("illegal type");
}

// Exposure state

Astro::ExposureState	convert(astro::camera::Exposure::State fromstate) {
	switch (fromstate) {
	case astro::camera::Exposure::idle:
		return Astro::EXPOSURE_IDLE;
	case astro::camera::Exposure::exposing:
		return Astro::EXPOSURE_EXPOSING;
	case astro::camera::Exposure::exposed:
		return Astro::EXPOSURE_EXPOSED;
	case astro::camera::Exposure::cancelling:
		return Astro::EXPOSURE_CANCELLING;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "illegal type: %d", fromstate);
	throw std::runtime_error("illegal type");
}

astro::camera::Exposure::State	convert(Astro::ExposureState fromstate) {
	switch (fromstate) {
	case Astro::EXPOSURE_IDLE:
		return astro::camera::Exposure::idle;
	case Astro::EXPOSURE_EXPOSING:
		return astro::camera::Exposure::exposing;
	case Astro::EXPOSURE_EXPOSED:
		return astro::camera::Exposure::exposed;
	case Astro::EXPOSURE_CANCELLING:
		return astro::camera::Exposure::cancelling;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "illegal type: %d", fromstate);
	throw std::runtime_error("illegal type");
}

std::string	convert2string(astro::camera::Exposure::State fromstate) {
        switch (fromstate) {
        case astro::camera::Exposure::idle:
                return std::string("IDLE");
        case astro::camera::Exposure::exposing:
                return std::string("EXPOSING");
        case astro::camera::Exposure::exposed:
                return std::string("EXPOSED");
        case astro::camera::Exposure::cancelling:
                return std::string("CANCELLING");
        }
        debug(LOG_ERR, DEBUG_LOG, 0, "illegal type: %d", fromstate);
        throw std::runtime_error("illegal type");
}

std::string	convert2string(Astro::ExposureState fromstate) {
	switch (fromstate) {
	case Astro::EXPOSURE_IDLE:
		return std::string("idle");
	case Astro::EXPOSURE_EXPOSING:
		return std::string("exposing");
	case Astro::EXPOSURE_EXPOSED:
		return std::string("exposed");
	case Astro::EXPOSURE_CANCELLING:
		return std::string("cancelling");
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "illegal type: %d", fromstate);
	throw std::runtime_error("illegal type");
}


// Image point

astro::image::ImagePoint	convert(const Astro::ImagePoint& point) {
	return ImagePoint(point.x, point.y);
}

Astro::ImagePoint	convert(const astro::image::ImagePoint& point) {
	Astro::ImagePoint	result;
	result.x = point.x();
	result.y = point.y();
	return result;
}

// generic Point

astro::Point	convert(const Astro::Point& point) {
	return astro::Point(point.x, point.y);
}

Astro::Point	convert(const astro::Point& point) {
	Astro::Point	result;
	result.x = point.x();
	result.y = point.y();
	return result;
}

// Image size

astro::image::ImageSize	convert(const Astro::ImageSize& size) {
	return ImageSize(size.width, size.height);
}

Astro::ImageSize	convert(const astro::image::ImageSize& size) {
	Astro::ImageSize	result;
	result.width = size.width();
	result.height = size.height();
	return result;
}

// Image rectangle

astro::image::ImageRectangle	convert(const Astro::ImageRectangle& rectangle) {
	return astro::image::ImageRectangle(convert(rectangle.origin),
						convert(rectangle.size));
}

Astro::ImageRectangle	convert(const astro::image::ImageRectangle& rectangle) {
	Astro::ImageRectangle	result;
	result.origin = convert(rectangle.origin());
	result.size = convert(rectangle.size());
	return result;
}

// shutter state

astro::camera::shutter_state    convert(const Astro::ShutterState& state) {
	switch (state) {
	case Astro::SHUTTER_OPEN:
		return astro::camera::SHUTTER_OPEN;
	case Astro::SHUTTER_CLOSED:
		return astro::camera::SHUTTER_CLOSED;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "unknown shutter state: %d", state);
	throw std::runtime_error("unknown shutter state");
}


Astro::ShutterState     convert(const astro::camera::shutter_state state) {
	switch (state) {
	case astro::camera::SHUTTER_OPEN:
		return Astro::SHUTTER_OPEN;
	case astro::camera::SHUTTER_CLOSED:
		return Astro::SHUTTER_CLOSED;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "unknown shutter state: %d", state);
	throw std::runtime_error("unknown shutter state");
}

// exposure purpose
astro::camera::Exposure::purpose_t        convert(const Astro::ExposurePurpose& purpose) {
	switch (purpose) {
	case Astro::PURPOSE_LIGHT:
		return astro::camera::Exposure::light;
	case Astro::PURPOSE_DARK:
		return astro::camera::Exposure::dark;
	case Astro::PURPOSE_FLAT:
		return astro::camera::Exposure::flat;
	}
	throw std::runtime_error("unknown exposure purpose");
}

Astro::ExposurePurpose convert(const astro::camera::Exposure::purpose_t& purpose) {
	switch (purpose) {
	case astro::camera::Exposure::light:
		return Astro::PURPOSE_LIGHT;
	case astro::camera::Exposure::dark:
		return Astro::PURPOSE_DARK;
	case astro::camera::Exposure::flat:
		return Astro::PURPOSE_FLAT;
	}
	throw std::runtime_error("unknown exposure purpose");
}


// Binning mode
astro::camera::Binning	convert(const Astro::BinningMode& mode) {
	return astro::camera::Binning(mode.x, mode.y);
}

Astro::BinningMode	convert(const astro::camera::Binning& mode) {
	Astro::BinningMode	result;
	result.x = mode.getX();
	result.y = mode.getY();
	return result;
}

// Binning Set
astro::camera::BinningSet	convert(Astro::BinningSet_var set) {
	astro::camera::BinningSet	result;
	for (unsigned int i = 0; i < set->length(); i++) {
		result.insert(convert(set[i]));
	}
	return result;
}

astro::camera::BinningSet	convert(Astro::BinningSet set) {
	astro::camera::BinningSet	result;
	for (unsigned int i = 0; i < set.length(); i++) {
		result.insert(convert(set[i]));
	}
	return result;
}

Astro::BinningSet_var	convert(const astro::camera::BinningSet& set) {
	Astro::BinningSet	*resultset = new Astro::BinningSet();
	Astro::BinningSet_var	result = resultset;
	resultset->length(set.size());
	astro::camera::BinningSet::const_iterator	i;
	unsigned int	j = 0;
	for (i = set.begin(); i != set.end(); i++, j++) {
		result[j] = convert(*i);
	}
	return result._retn();
}

// Exposure

astro::camera::Exposure	convert(const Astro::Exposure& exposure) {
	astro::camera::Exposure	result(convert(exposure.frame),
					exposure.exposuretime);
	result.gain = exposure.gain;
	result.limit = exposure.limit;
	if (exposure.limit > 0) {
		result.limit = exposure.limit;
	}
	result.shutter = convert(exposure.shutter);
	result.mode = convert(exposure.mode);
	return result;
}

Astro::Exposure	convert(const astro::camera::Exposure& exposure) {
	Astro::Exposure	result;
	result.frame = convert(exposure.frame);
	result.exposuretime = exposure.exposuretime;
	result.gain = exposure.gain;
	result.limit = exposure.limit;
	result.shutter = convert(exposure.shutter);
	result.mode = convert(exposure.mode);
	return result;
}

// relaybits
uint8_t	convert_octet2relaybits(CORBA::Octet bits) {
	uint8_t	result = 0;
	if (bits & Astro::GuiderPort::DECMINUS) {
		result |= astro::camera::GuiderPort::DECMINUS;
	}
	if (bits & Astro::GuiderPort::DECPLUS) {
		result |= astro::camera::GuiderPort::DECPLUS;
	}
	if (bits & Astro::GuiderPort::RAMINUS) {
		result |= astro::camera::GuiderPort::RAMINUS;
	}
	if (bits & Astro::GuiderPort::RAPLUS) {
		result |= astro::camera::GuiderPort::RAPLUS;
	}
	return result;
}

CORBA::Octet	convert_relaybits2octet(uint8_t bits) {
	CORBA::Octet	result = 0;
	if (bits & astro::camera::GuiderPort::DECMINUS) {
		result |= Astro::GuiderPort::DECMINUS;
	}
	if (bits & astro::camera::GuiderPort::DECPLUS) {
		result |= Astro::GuiderPort::DECPLUS;
	}
	if (bits & astro::camera::GuiderPort::RAMINUS) {
		result |= Astro::GuiderPort::RAMINUS;
	}
	if (bits & astro::camera::GuiderPort::RAPLUS) {
		result |= Astro::GuiderPort::RAPLUS;
	}
	return result;
}

// CcdInfo
Astro::CcdInfo	*convert(const astro::camera::CcdInfo& info) {
	Astro::CcdInfo	*result = new Astro::CcdInfo;
	std::string	ccdname = info.name();
	result->name = CORBA::string_dup(ccdname.c_str());
	result->id = info.getId();
	result->size = convert(info.size());
	result->binningmodes = convert(info.modes());
	result->shutter = info.shutter();
	result->pixelwidth = info.pixelwidth();
	result->pixelheight = info.pixelheight();
	return result;
}

astro::camera::CcdInfo	convert(const Astro::CcdInfo& info) {
	astro::camera::CcdInfo	result(std::string(info.name),
					convert(info.size), info.id);
	result.addModes(convert(info.binningmodes));
	result.setShutter(info.shutter);
	result.pixelwidth(info.pixelwidth);
	result.pixelheight(info.pixelheight);
	return result;
}

astro::camera::CcdInfo	convert(const Astro::CcdInfo_var& info) {
	astro::camera::CcdInfo	result(std::string(info->name),
					convert(info->size), info->id);
	result.addModes(convert(info->binningmodes));
	result.setShutter(info->shutter);
	result.pixelwidth(info->pixelwidth);
	result.pixelheight(info->pixelheight);
	return result;
}

// GuiderDescriptor

astro::guiding::GuiderDescriptor	convert(
	const Astro::GuiderDescriptor& guiderdescriptor) {
	astro::guiding::GuiderDescriptor	result(
		std::string(guiderdescriptor.cameraname),
		guiderdescriptor.ccdid,
		std::string(guiderdescriptor.guiderportname)
	);
	return result;
}

Astro::GuiderDescriptor	convert(
	const astro::guiding::GuiderDescriptor& guiderdescriptor) {
	Astro::GuiderDescriptor	result;
	result.cameraname = CORBA::string_dup(
		guiderdescriptor.cameraname().c_str());
	result.ccdid = guiderdescriptor.ccdid();
	result.guiderportname = CORBA::string_dup(
		guiderdescriptor.guiderportname().c_str());
	return result;
}

// Filterwheel state
astro::camera::FilterWheel::State       convert(const Astro::FilterwheelState& state) {
	switch (state) {
	case Astro::FILTERWHEEL_IDLE:
		return astro::camera::FilterWheel::idle;
	case Astro::FILTERWHEEL_MOVING:
		return astro::camera::FilterWheel::moving;
	case Astro::FILTERWHEEL_UNKNOWN:
		return astro::camera::FilterWheel::unknown;
	}
	throw std::runtime_error("unknown filter wheel state");
}

Astro::FilterwheelState convert(const astro::camera::FilterWheel::State& state) {
	switch (state) {
	case astro::camera::FilterWheel::idle:
		return Astro::FILTERWHEEL_IDLE;
	case astro::camera::FilterWheel::moving:
		return Astro::FILTERWHEEL_MOVING;
	case astro::camera::FilterWheel::unknown:
		return Astro::FILTERWHEEL_UNKNOWN;
	}
	throw std::runtime_error("unknown filter wheel state");
}

// GuiderCalibration
astro::guiding::GuiderCalibration       convert(const Astro::Calibration& cal) {
	return astro::guiding::GuiderCalibration(cal.coefficients);
}

// TaskState
astro::task::TaskQueueEntry::taskstate  convert(const Astro::TaskState& state) {
	switch (state) {
	case Astro::TASK_PENDING:
		return astro::task::TaskQueueEntry::pending;
	case Astro::TASK_EXECUTING:
		return astro::task::TaskQueueEntry::executing;
	case Astro::TASK_FAILED:
		return astro::task::TaskQueueEntry::failed;
	case Astro::TASK_CANCELLED:
		return astro::task::TaskQueueEntry::cancelled;
	case Astro::TASK_COMPLETED:
		return astro::task::TaskQueueEntry::complete;
	}
	throw std::runtime_error("unknown state");
}

Astro::TaskState        convert(const astro::task::TaskQueueEntry::taskstate& state) {
	switch (state) {
	case astro::task::TaskQueueEntry::pending:
		return Astro::TASK_PENDING;
	case astro::task::TaskQueueEntry::executing:
		return Astro::TASK_EXECUTING;
	case astro::task::TaskQueueEntry::failed:
		return Astro::TASK_FAILED;
	case astro::task::TaskQueueEntry::cancelled:
		return Astro::TASK_CANCELLED;
	case astro::task::TaskQueueEntry::complete:
		return Astro::TASK_COMPLETED;
	}
	throw std::runtime_error("unknown state");
}

// TaskQueueState
astro::task::TaskQueue::state_type
convert(const Astro::TaskQueue::QueueState state) {
	switch (state) {
	case Astro::TaskQueue::IDLE:
		return astro::task::TaskQueue::idle;
	case Astro::TaskQueue::LAUNCHING:
		return astro::task::TaskQueue::launching;
	case Astro::TaskQueue::STOPPING:
		return astro::task::TaskQueue::stopping;
	case Astro::TaskQueue::STOPPED:
		return astro::task::TaskQueue::stopped;
	}
	throw std::runtime_error("unknown state");
}

Astro::TaskQueue::QueueState
convert(const astro::task::TaskQueue::state_type state) {
	switch (state) {
	case astro::task::TaskQueue::idle:
		return Astro::TaskQueue::IDLE;
	case astro::task::TaskQueue::launching:
		return Astro::TaskQueue::LAUNCHING;
	case astro::task::TaskQueue::stopping:
		return Astro::TaskQueue::STOPPING;
	case astro::task::TaskQueue::stopped:
		return Astro::TaskQueue::STOPPED;
	}
	throw std::runtime_error("unknown state");
}

// Task parameters
astro::task::TaskParameters	convert(const Astro::TaskParameters& parameters) {
	astro::camera::Exposure	exposure;

	// origin
	astro::image::ImagePoint	origin(parameters.exp.frame.origin.x,
				parameters.exp.frame.origin.y);
	exposure.frame.setOrigin(origin);

	// size
	astro::image::ImageSize	size(parameters.exp.frame.size.width,
				parameters.exp.frame.size.height);
	exposure.frame.setSize(size);

	// binning mode
	astro::camera::Binning	mode(parameters.exp.mode.x,
					parameters.exp.mode.y);
	exposure.mode = mode;

	exposure.exposuretime = parameters.exp.exposuretime;
	exposure.gain = parameters.exp.gain;
	exposure.limit = parameters.exp.limit;

	switch (parameters.exp.shutter) {
	case Astro::SHUTTER_CLOSED:
		exposure.shutter = astro::camera::SHUTTER_CLOSED;
		break;
	case Astro::SHUTTER_OPEN:
		exposure.shutter = astro::camera::SHUTTER_OPEN;
		break;
	}

	// remaining parameters
	astro::task::TaskParameters	task;
	task.exposure(exposure);
	task.camera(std::string(parameters.camera));
	task.ccdid(parameters.ccdid);
	task.ccdtemperature(parameters.ccdtemperature);
	task.filterwheel(std::string(parameters.filterwheel));
	task.filterposition(parameters.filterposition);

	return task;
}

Astro::TaskParameters	convert(const astro::task::TaskParameters& task) {
	Astro::TaskParameters	parameters;
	parameters.camera = CORBA::string_dup(task.camera().c_str());
	parameters.ccdid = task.ccdid();
	parameters.ccdtemperature = task.ccdtemperature();
	parameters.filterwheel = CORBA::string_dup(task.filterwheel().c_str());
	parameters.filterposition = task.filterposition();
	//astro::camera::Exposure	exposure = task.exposure();
	parameters.exp.exposuretime = task.exposure().exposuretime;
	parameters.exp.gain = task.exposure().gain;
	parameters.exp.limit = task.exposure().limit;
	parameters.exp.mode.x = task.exposure().mode.getX();
	parameters.exp.mode.y = task.exposure().mode.getY();
	parameters.exp.shutter = astro::convert(task.exposure().shutter);
	parameters.exp.frame = astro::convert(task.exposure().frame);
	return parameters;
}

// Task info, note that the resulting task info structure is necessarily
// very incomplete
astro::task::TaskInfo     convert(const Astro::TaskInfo& info) {
	astro::task::TaskInfo	entry(info.taskid);
	entry.state(convert(info.state));
	time_t	now = time(NULL);
	entry.lastchange(now - info.lastchange);
	entry.cause(std::string(info.cause));
	entry.filename(std::string(info.filename));
	entry.origin(ImagePoint(info.frame.origin.x, info.frame.origin.y));
	entry.size(ImageSize(info.frame.size.width, info.frame.size.height));
	return entry;
}

Astro::TaskInfo convert(const astro::task::TaskInfo& task) {
	Astro::TaskInfo	info;
	info.taskid = task.id();
	info.state = astro::convert(task.state());
	time_t	now = time(NULL);
	info.lastchange = now - task.lastchange();
	info.cause = CORBA::string_dup(task.cause().c_str());
	info.filename = CORBA::string_dup(task.filename().c_str());
	info.frame.origin.x = task.origin().x();
	info.frame.origin.y = task.origin().y();
	info.frame.size.width = task.size().width();
	info.frame.size.height = task.size().height();
	return info;
}

// Guider State
astro::guiding::GuiderState     convert(const Astro::Guider::GuiderState& state) {
	switch (state) {
	case Astro::Guider::GUIDER_UNCONFIGURED:
		return astro::guiding::unconfigured;
	case Astro::Guider::GUIDER_IDLE:
		return astro::guiding::idle;
	case Astro::Guider::GUIDER_CALIBRATING:
		return astro::guiding::calibrating;
	case Astro::Guider::GUIDER_CALIBRATED:
		return astro::guiding::calibrated;
	case Astro::Guider::GUIDER_GUIDING:
		return astro::guiding::guiding;
	}
	throw std::runtime_error("unknown state");
}

Astro::Guider::GuiderState      convert(const astro::guiding::GuiderState& state) {
	switch (state) {
	case astro::guiding::unconfigured:
		return Astro::Guider::GUIDER_UNCONFIGURED;
	case astro::guiding::idle:
		return Astro::Guider::GUIDER_IDLE;
	case astro::guiding::calibrating:
		return Astro::Guider::GUIDER_CALIBRATING;
	case astro::guiding::calibrated:
		return Astro::Guider::GUIDER_CALIBRATED;
	case astro::guiding::guiding:
		return Astro::Guider::GUIDER_GUIDING;
	}
	throw std::runtime_error("unknown state");
}

// Tracking info
astro::guiding::TrackingPoint    convert(const Astro::TrackingPoint& trackinginfo) {
	astro::guiding::TrackingPoint	result(
		astro::Timer::gettime()  - trackinginfo.timeago,
		convert(trackinginfo.trackingoffset),
		convert(trackinginfo.activation));
	return result;
}

Astro::TrackingPoint    convert(const astro::guiding::TrackingPoint& trackinginfo) {
	Astro::TrackingPoint	result;
	result.timeago = astro::Timer::gettime() - trackinginfo.t;
	result.trackingoffset = convert(trackinginfo.trackingoffset);
	result.activation = convert(trackinginfo.correction);
	return result;
}

// CalibrationPoint
astro::guiding::CalibrationPoint	convert(const Astro::CalibrationPoint& calibrationpoint) {
	astro::guiding::CalibrationPoint	result;
	result.t = calibrationpoint.t;
	result.offset = convert(calibrationpoint.offset);
	result.star = convert(calibrationpoint.star);
	return result;
}

Astro::CalibrationPoint	convert(const astro::guiding::CalibrationPoint& calibrationpoint) {
	Astro::CalibrationPoint	result;
	result.t = calibrationpoint.t;
	result.offset = convert(calibrationpoint.offset);
	result.star = convert(calibrationpoint.star);
	return result;
}

// TaskMonitorInfo
astro::task::TaskMonitorInfo	convert(const Astro::TaskMonitorInfo& tmi) {
	astro::task::TaskMonitorInfo	taskmonitorinfo;
	taskmonitorinfo.taskid(tmi.taskid);
	taskmonitorinfo.when(Timer::gettime() - tmi.timeago);
	taskmonitorinfo.state(convert(tmi.newstate));
	return taskmonitorinfo;
}

Astro::TaskMonitorInfo	convert(const astro::task::TaskMonitorInfo& tmi) {
	Astro::TaskMonitorInfo	taskmonitorinfo;
	taskmonitorinfo.taskid = tmi.taskid();
	taskmonitorinfo.timeago = Timer::gettime() - tmi.when();
	taskmonitorinfo.newstate = convert(tmi.state());
	return taskmonitorinfo;
}

} // namespace astro
