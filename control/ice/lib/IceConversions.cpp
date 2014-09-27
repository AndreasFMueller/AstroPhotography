/*
 * IceConversions.cpp -- conversions between ice and astro
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <IceConversions.h>
#include <includes.h>
#include <AstroIO.h>
#include <AstroUtils.h>

namespace snowstar {

// time conversions
time_t	converttime(double timeago) {
	time_t	now;
	time(&now);
	time_t	result = now - timeago;
	return result;
}

double	converttime(time_t t) {
	time_t	now;
	time(&now);
	return now - t;
}
 
struct timeval	converttimeval(double timeago) {
	struct timeval	result;
	gettimeofday(&result, NULL);
	result.tv_sec -= (int)trunc(timeago);
	result.tv_usec -= (int)(1000000 * (timeago - trunc(timeago)));
	while (result.tv_usec < 0) {
		result.tv_sec -= 1;
		result.tv_usec += 1000000;
	}
	while (result.tv_usec > 1000000) {
		result.tv_sec += 1;
		result.tv_usec -= 1000000;
	}
	return result;
}

double	converttimeval(struct timeval t) {
	struct timeval	now;
	gettimeofday(&now, NULL);
	return (now.tv_sec - t.tv_sec) + (now.tv_usec - t.tv_usec) / 1000000.;
}


// Types
ImagePoint	convert(const astro::image::ImagePoint& point) {
	ImagePoint	result;
	result.x = point.x();
	result.y = point.y();
	return result;
}

astro::image::ImagePoint	convert(const ImagePoint& point) {
	return astro::image::ImagePoint(point.x, point.y);
}

ImageSize	convert(const astro::image::ImageSize& size) {
	ImageSize	result;
	result.width = size.width();
	result.height = size.height();
	return result;
}

astro::image::ImageSize	convert(const ImageSize& size) {
	return astro::image::ImageSize(size.width, size.height);
}

ImageRectangle	convert(const astro::image::ImageRectangle& rectangle) {
	ImageRectangle	result;
	result.origin = convert(rectangle.origin());
	result.size = convert(rectangle.size());
	return result;
}

astro::image::ImageRectangle	convert(const ImageRectangle& rectangle) {
	return astro::image::ImageRectangle(convert(rectangle.origin),
			convert(rectangle.size));
}

Point	convert(const astro::Point& point) {
	Point	result;
	result.x = point.x();
	result.y = point.y();
	return result;
}

astro::Point	convert(const Point& point) {
	return astro::Point(point.x, point.y);
}

astro::RaDec	convert(const RaDec& radec) {
	astro::Angle	ra;
	ra.hours(radec.ra);
	astro::Angle	dec;
	dec.degrees(radec.dec);
	return astro::RaDec(ra, dec);
}

RaDec	convert(const astro::RaDec& radec) {
	RaDec	result;
	result.ra = radec.ra().hours();
	result.dec = radec.dec().degrees();
	return result;
}

astro::AzmAlt	convert(const AzmAlt& azmalt) {
	astro::Angle	azm;
	azm.degrees(azmalt.azm);
	astro::Angle	alt;
	alt.degrees(azmalt.alt);
	return astro::AzmAlt(azm, alt);
}

AzmAlt	convert(const astro::AzmAlt& azmalt) {
	AzmAlt	result;
	result.azm = azmalt.azm().degrees();
	result.alt = azmalt.alt().degrees();
	return result;
}

// Device conversions
DeviceNameList	convert(const astro::module::Devices::devicelist& list) {
	DeviceNameList	result;
	for (auto ptr = list.begin(); ptr != list.end(); ptr++) {
		result.push_back(ptr->toString());
	}
	return result;
}

astro::module::Devices::devicelist	convert(const DeviceNameList& list) {
	astro::module::Devices::devicelist	result;
	for (auto ptr = list.begin(); ptr != list.end(); ptr++) {
		result.push_back(astro::DeviceName(*ptr));
	}
	return result;
}

devicetype	convert(const astro::DeviceName::device_type& type) {
	switch (type) {
	case astro::DeviceName::AdaptiveOptics:
		return DevAO;
	case astro::DeviceName::Camera:
		return DevCAMERA;
	case astro::DeviceName::Ccd:
		return DevCCD;
	case astro::DeviceName::Cooler:
		return DevCOOLER;;
	case astro::DeviceName::Filterwheel:
		return DevFILTERWHEEL;
	case astro::DeviceName::Focuser:
		return DevFOCUSER;
	case astro::DeviceName::Guiderport:
		return DevGUIDERPORT;
	case astro::DeviceName::Module:
		return DevMODULE;
	case astro::DeviceName::Mount:
		return DevMOUNT;
	}
}

astro::DeviceName::device_type	convert(const devicetype& type) {
	switch (type) {
	case DevAO:
		return astro::DeviceName::AdaptiveOptics;
	case DevCAMERA:
		return astro::DeviceName::Camera;
	case DevCCD:
		return astro::DeviceName::Ccd;
	case DevCOOLER:
		return astro::DeviceName::Cooler;
	case DevFILTERWHEEL:
		return astro::DeviceName::Filterwheel;
	case DevFOCUSER:
		return astro::DeviceName::Focuser;
	case DevGUIDERPORT:
		return astro::DeviceName::Guiderport;
	case DevMODULE:
		return astro::DeviceName::Module;
	case DevMOUNT:
		return astro::DeviceName::Mount;
	}
}

// CCD related
BinningMode	convert(const astro::camera::Binning& binning) {
	BinningMode	result;
	result.x = binning.getX();
	result.y = binning.getY();
	return result;
}

astro::camera::Binning	convert(const BinningMode& mode) {
	return astro::camera::Binning(mode.x, mode.y);
}

BinningSet	convert(const astro::camera::BinningSet& binningset) {
	BinningSet	result;
	for (auto ptr = binningset.begin(); ptr != binningset.end(); ptr++) {
		result.push_back(convert(*ptr));
	}
	return result;
}

astro::camera::BinningSet	convert(const BinningSet& binningset) {
	astro::camera::BinningSet	result;
	for (auto ptr = binningset.begin(); ptr != binningset.end(); ptr++) {
		result.insert(convert(*ptr));
	}
	return result;
}

CcdInfo	convert(const astro::camera::CcdInfo& info) {
	CcdInfo	result;
	result.name = info.name();
	result.id = info.getId();
	result.size.width = info.size().width();
	result.size.height = info.size().height();
	result.shutter = info.shutter();
	result.pixelheight = info.pixelheight();
	result.pixelwidth = info.pixelwidth();
	result.binningmodes = convert(info.modes());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccdinfo has %d binning modes",
		result.binningmodes.size());
	return result;
}

astro::camera::CcdInfo	convert(const CcdInfo& info) {
	astro::camera::CcdInfo	result(info.name, convert(info.size), info.id);
	result.setShutter(info.shutter);
	result.pixelwidth(info.pixelwidth);
	result.pixelheight(info.pixelheight);
	result.addModes(convert(info.binningmodes));
	return result;
}

Exposure	convert(const astro::camera::Exposure& exp) {
	Exposure	exposure;
	exposure.frame = convert(exp.frame);
	exposure.exposuretime = exp.exposuretime;
	exposure.gain = exp.gain;
	exposure.limit = exp.limit;
	exposure.shutter = convert(exp.shutter);
	exposure.purpose = convert(exp.purpose);
	exposure.mode = convert(exp.mode);
	return exposure;
}

astro::camera::Exposure	convert(const Exposure& exposure) {
	astro::camera::Exposure	exp;
	exp.frame = convert(exposure.frame);
	exp.exposuretime = exposure.exposuretime;
	exp.gain = exposure.gain;
	exp.limit = exposure.limit;
	exp.shutter = convert(exposure.shutter);
	exp.purpose = convert(exposure.purpose);
	exp.mode = convert(exposure.mode);
	return exp;
}

ExposureState	convert(const astro::camera::Exposure::State& s) {
	switch (s) {
	case astro::camera::Exposure::idle:
		return IDLE;
	case astro::camera::Exposure::exposing:
		return EXPOSING;
	case astro::camera::Exposure::cancelling:
		return CANCELLING;
	case astro::camera::Exposure::exposed:
		return EXPOSED;
	}
}

astro::camera::Exposure::State	convert(const ExposureState& s) {
	switch (s) {
	case IDLE:
		return astro::camera::Exposure::idle;
	case EXPOSING:
		return astro::camera::Exposure::exposing;
	case CANCELLING:
		return astro::camera::Exposure::cancelling;
	case EXPOSED:
		return astro::camera::Exposure::exposed;
	}
}

ShutterState	convert(const astro::camera::shutter_state& s) {
	ShutterState	result = snowstar::ShOPEN;
	switch (s) {
	case astro::camera::SHUTTER_OPEN:
		result = snowstar::ShOPEN;
		break;
	case astro::camera::SHUTTER_CLOSED:
		result = snowstar::ShCLOSED;
		break;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s",
		(s == astro::camera::SHUTTER_OPEN) ? "open" : "closed",
		(result == ShOPEN) ? "open" : "closed");
	return result;
}

astro::camera::shutter_state	convert(const ShutterState& s) {
	astro::camera::shutter_state	result = astro::camera::SHUTTER_OPEN;
	switch (s) {
	case snowstar::ShOPEN:
		result = astro::camera::SHUTTER_OPEN;
		break;
	case snowstar::ShCLOSED:
		result = astro::camera::SHUTTER_CLOSED;
		break;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s",
		(s == ShOPEN) ? "open" : "closed",
		(result == astro::camera::SHUTTER_OPEN) ? "open" : "closed");
	return result;
}

ExposurePurpose convert(const astro::camera::Exposure::purpose_t& purpose) {
	switch (purpose) {
	case astro::camera::Exposure::light:
		return snowstar::ExLIGHT;
	case astro::camera::Exposure::dark:
		return snowstar::ExDARK;
	case astro::camera::Exposure::flat:
		return snowstar::ExFLAT;
	}
	throw std::runtime_error("unknown exposure purpose");
}

astro::camera::Exposure::purpose_t      convert(const ExposurePurpose& purpose) {
	switch (purpose) {
	case snowstar::ExLIGHT:
		return astro::camera::Exposure::light;
	case snowstar::ExDARK:
		return astro::camera::Exposure::dark;
	case snowstar::ExFLAT:
		return astro::camera::Exposure::flat;
	}
	throw std::runtime_error("unknown exposure purpose");
}

// Filterwheel
FilterwheelState	convert(const astro::camera::FilterWheel::State& s) {
	switch (s) {
	case astro::camera::FilterWheel::idle:
		return snowstar::FwIDLE;
	case astro::camera::FilterWheel::moving:
		return snowstar::FwMOVING;
	case astro::camera::FilterWheel::unknown:
		return snowstar::FwUNKNOWN;
	}
}

astro::camera::FilterWheel::State convert(const FilterwheelState& s) {
	switch (s) {
	case snowstar::FwIDLE:
		return astro::camera::FilterWheel::idle;
	case snowstar::FwMOVING:
		return astro::camera::FilterWheel::moving;
	case snowstar::FwUNKNOWN:
		return astro::camera::FilterWheel::unknown;
	}
}

// Guider
GuiderState	convert(const astro::guiding::GuiderState& state) {
	switch (state) {
	case astro::guiding::unconfigured:
		return GuiderUNCONFIGURED;
	case astro::guiding::idle:
		return GuiderIDLE;
	case astro::guiding::calibrating:
		return GuiderCALIBRATING;
	case astro::guiding::calibrated:
		return GuiderCALIBRATED;
	case astro::guiding::guiding:
		return GuiderGUIDING;
	}
}

astro::guiding::GuiderState	convert(const GuiderState& state) {
	switch (state) {
	case GuiderUNCONFIGURED:
		return astro::guiding::unconfigured;
	case GuiderIDLE:
		return astro::guiding::idle;
	case GuiderCALIBRATING:
		return astro::guiding::calibrating;
	case GuiderCALIBRATED:
		return astro::guiding::calibrated;
	case GuiderGUIDING:
		return astro::guiding::guiding;
	}
}

GuiderDescriptor        convert(const astro::guiding::GuiderDescriptor& gd) {
	GuiderDescriptor	result;
	result.cameraname = gd.cameraname();
	result.ccdid = gd.ccdid();
	result.guiderportname = gd.guiderportname();
	return result;
}

astro::guiding::GuiderDescriptor        convert(const GuiderDescriptor& gd) {
	astro::guiding::GuiderDescriptor	result(gd.cameraname,
					gd.ccdid, gd.guiderportname);
	return result;
}

TrackingPoint   convert(const astro::guiding::TrackingPoint& trackingpoint) {
	TrackingPoint	result;
	result.timeago = converttime(trackingpoint.t);
	result.trackingoffset = convert(trackingpoint.trackingoffset);
	result.activation = convert(trackingpoint.correction);
	return result;
}

astro::guiding::TrackingPoint   convert(const TrackingPoint& trackingpoint) {
	astro::guiding::TrackingPoint	result;
	result.t = converttime(trackingpoint.timeago);
	result.trackingoffset = convert(trackingpoint.trackingoffset);
	result.correction = convert(trackingpoint.activation);
	return result;
}

CalibrationPoint        convert(const astro::guiding::CalibrationPoint& cp) {
	CalibrationPoint	result;
	result.t = cp.t;
	result.offset = convert(cp.offset);
	result.star = convert(cp.star);
	return result;
}

astro::guiding::CalibrationPoint        convert(const CalibrationPoint& cp) {
	astro::guiding::CalibrationPoint	result;
	result.t = cp.t;
	result.offset = convert(cp.offset);
	result.star = convert(cp.star);
	return result;
}

// TaskQueue
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

astro::task::TaskInfo	convert(const TaskInfo& info) {
	astro::task::TaskInfo	result(info.taskid);
	result.state(convert(info.state));
	result.lastchange(info.lastchange);
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

// Mount
mountstate	convert(astro::device::Mount::mount_state s) {
	switch (s) {
	case astro::device::Mount::IDLE:
		return MountIDLE;
	case astro::device::Mount::TRACKING:
		return MountTRACKING;
	case astro::device::Mount::GOTO:
		return MountGOTO;
	}
	throw std::runtime_error("unknown state");
}

astro::device::Mount::mount_state	convert(mountstate s) {
	switch (s) {
	case MountIDLE:
		return astro::device::Mount::IDLE;
	case MountTRACKING:
		return astro::device::Mount::TRACKING;
	case MountGOTO:
		return astro::device::Mount::GOTO;
	}
	throw std::runtime_error("unknown state");
}

// Image
astro::image::ImagePtr	convert(ImagePrx image) {
	// get the image data from the server
	ImageFile	file = image->file();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got image of size %d", file.size());

	// construct a temporary file name
	char	buffer[1024];
	if (getenv("TMPDIR")) {
		snprintf(buffer, sizeof(buffer), "%s/convert-XXXXXX.fits",
			getenv("TMPDIR"));
	} else {
		strcpy(buffer, "/tmp/convert-XXXXXX.fits");
	}
	if (mkstemps(buffer, 5) < 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot create temp file: %s",
			strerror(errno));
		throw std::runtime_error("cannot create tmp file name");
	}
	std::string	filename(buffer);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "temporary image file: %s", buffer);

	// write image data to a temporary file
	int	out = open(buffer, O_CREAT | O_WRONLY, 0666);
	if (out < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot create temporary file %s",
			buffer);
		throw std::runtime_error("cannot create temp file");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "out fd = %d", out);
	if (file.size() != write(out, file.data(), file.size())) {
		debug(LOG_ERR, DEBUG_LOG, 0, "writing temp file failed: %s",
			strerror(errno));
	}
	close(out);

	// use FITS classes to read the temporary file
	astro::io::FITSin	in(filename);
	astro::image::ImagePtr	result = in.read();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got an %s image with pixel type %s",
		result->size().toString().c_str(),
		astro::demangle(result->pixel_type().name()).c_str());

	// unlink the temporary file
	unlink(filename.c_str());

	// return the image we just read
	return result;
}

} // namespace snowstar
