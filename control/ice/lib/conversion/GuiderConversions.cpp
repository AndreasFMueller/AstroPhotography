/*
 * GuiderConversions.cpp -- conversions between ice and astro
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <IceConversions.h>
#include <includes.h>
#include <AstroIO.h>
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <iostream>
#include <sstream>
#include <Backlash.h>

namespace snowstar {

GuidePortActivation	convert(const astro::camera::GuidePortActivation& a) {
	GuidePortActivation	aa;
	aa.raplus = a.raplus();
	aa.raminus = a.raminus();
	aa.decplus = a.decplus();
	aa.decminus = a.decminus();
	return aa;
}

astro::camera::GuidePortActivation	convert(const GuidePortActivation& a) {
	return astro::camera::GuidePortActivation(a.raplus, a.raminus,
		a.decplus, a.decminus);
}

GuiderState	convert(const astro::guiding::Guide::state& state) {
	switch (state) {
	case astro::guiding::Guide::unconfigured:
		return GuiderUNCONFIGURED;
	case astro::guiding::Guide::idle:
		return GuiderIDLE;
	case astro::guiding::Guide::calibrating:
		return GuiderCALIBRATING;
	case astro::guiding::Guide::calibrated:
		return GuiderCALIBRATED;
	case astro::guiding::Guide::guiding:
		return GuiderGUIDING;
	case astro::guiding::Guide::darkacquire:
		return GuiderDARKACQUIRE;
	case astro::guiding::Guide::flatacquire:
		return GuiderFLATACQUIRE;
	case astro::guiding::Guide::imaging:
		return GuiderIMAGING;
	case astro::guiding::Guide::backlash:
		return GuiderBACKLASH;
	}
	throw std::runtime_error("unknown guider state");
}

astro::guiding::Guide::state	convert(const GuiderState& state) {
	switch (state) {
	case GuiderUNCONFIGURED:
		return astro::guiding::Guide::unconfigured;
	case GuiderIDLE:
		return astro::guiding::Guide::idle;
	case GuiderCALIBRATING:
		return astro::guiding::Guide::calibrating;
	case GuiderCALIBRATED:
		return astro::guiding::Guide::calibrated;
	case GuiderGUIDING:
		return astro::guiding::Guide::guiding;
	case GuiderDARKACQUIRE:
		return astro::guiding::Guide::darkacquire;
	case GuiderFLATACQUIRE:
		return astro::guiding::Guide::flatacquire;
	case GuiderIMAGING:
		return astro::guiding::Guide::imaging;
	case GuiderBACKLASH:
		return astro::guiding::Guide::backlash;
	}
	throw std::runtime_error("unknown guider state");
}

GuiderState	string2guiderstate(const std::string& s) {
	return convert(astro::guiding::Guide::string2state(s));
}

std::string	guiderstate2string(GuiderState state) {
	return astro::guiding::Guide::state2string(convert(state));
}

std::string        convert(const astro::guiding::GuiderDescriptor& gd) {
	return gd.instrument();
}

/**
 * \brief Convert a guider descriptor into a locally useful guider name
 */
astro::guiding::GuiderDescriptor        convertGuiderDescriptor(const std::string& instrumentname) {
	// construct the guider descriptor
	astro::guiding::GuiderDescriptor	result(instrumentname);
	return result;
}

std::string        convertname(const astro::guiding::GuiderName& name) {
	std::string	result = name.instrument();
	return result;
}

astro::guiding::GuiderName      convertnameGuiderName(const std::string& name) {
	return astro::guiding::GuiderName(name);
}

TrackingPoint   convert(const astro::guiding::TrackingPoint& trackingpoint) {
	TrackingPoint	result;
	result.timeago = converttime(trackingpoint.t);
	result.trackingoffset = convert(trackingpoint.trackingoffset);
	result.activation = convert(trackingpoint.correction);
	switch (trackingpoint.type) {
	case astro::guiding::GP:
		result.type = ControlGuidePort;
		break;
	case astro::guiding::AO:
		result.type = ControlAdaptiveOptics;
		break;
	}
	return result;
}

astro::guiding::TrackingPoint   convert(const TrackingPoint& trackingpoint) {
	astro::guiding::TrackingPoint	result;
	result.t = converttime(trackingpoint.timeago);
	result.trackingoffset = convert(trackingpoint.trackingoffset);
	result.correction = convert(trackingpoint.activation);
	switch (trackingpoint.type) {
	case ControlGuidePort:
		result.type = astro::guiding::GP;
		break;
	case ControlAdaptiveOptics:
		result.type = astro::guiding::AO;
		break;
	}
	return result;
}

astro::guiding::TrackingHistory	convert(const TrackingHistory& history) {
	astro::guiding::TrackingHistory	result;
	result.instrument = history.instrument;
	result.guideportcalid = history.guideportcalid;
	result.adaptiveopticscalid = history.adaptiveopticscalid;
#if 0
	result.ccd = instrumentIndex2name(result.instrument)
		InstrumentGuiderCCD, history.guider.ccdIndex);
	if (history.guider.guideportIndex >= 0) {
		result.guideport = instrumentIndex2name(result.instrument,
			InstrumentGuidePort,
			history.guider.guideportIndex);
	}
	if (history.guider.adaptiveopticsIndex >= 0) {
		result.adaptiveoptics = instrumentIndex2name(result.instrument,
			InstrumentAdaptiveOptics,
			history.guider.adaptiveopticsIndex);
	}
#endif
	result.whenstarted = converttime(history.timeago);
	for (auto ptr = history.points.begin(); ptr != history.points.end();
		ptr++) {
		result.points.push_back(convert(*ptr));
	}
	return result;
}

TrackingHistory	convert(const astro::guiding::TrackingHistory& history) {
	TrackingHistory	result;
	result.trackid = history.trackid;
	result.instrument = history.instrument;
	result.guideportcalid = history.guideportcalid;
	result.adaptiveopticscalid = history.adaptiveopticscalid;
	result.instrument = history.instrument;
#if 0
	result.guider.ccdIndex = instrumentName2index(history.instrument,
		InstrumentGuiderCCD, history.ccd);
	result.guider.guideportIndex = -1;
	if (history.guideport.size()) {
		result.guider.guideportIndex = instrumentName2index(
			history.instrument, InstrumentGuidePort,
			history.guideport);
	}
	result.guider.adaptiveopticsIndex = -1;
	if (history.adaptiveoptics.size()) {
		result.guider.adaptiveopticsIndex = instrumentName2index(
			history.instrument, InstrumentAdaptiveOptics,
			history.adaptiveoptics);
	}
#endif
	result.timeago = converttime(history.whenstarted);
	for (auto ptr = history.points.begin(); ptr != history.points.end();
		ptr++) {
		result.points.push_back(convert(*ptr));
	}
	return result;
}

ControlType     convertcontroltype(
	const astro::guiding::ControlDeviceType& caltype) {
	switch (caltype) {
	case astro::guiding::GP:
		return ControlGuidePort;
	case astro::guiding::AO:
		return ControlAdaptiveOptics;
	}
	throw std::runtime_error("unknown control device type");
}

astro::guiding::ControlDeviceType       convertcontroltype(
	const ControlType& caltype) {
	switch (caltype) {
	case ControlGuidePort:
		return astro::guiding::GP;
	case ControlAdaptiveOptics:
		return astro::guiding::AO;
	}
	throw std::runtime_error("unknown control device type");
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

Calibration     convert(const astro::guiding::CalibrationPtr cal) {
	Calibration	result;
	result.id = cal->calibrationid();
	result.timeago = converttime(cal->when());
	result.east = (cal->east()) ? 1 : 0;
	result.declination = cal->declination().degrees();
	result.instrument = convertname(cal->name());
	result.type = convertcontroltype(cal->calibrationtype());
	result.focallength = cal->focallength();
	result.masPerPixel = cal->masPerPixel();
	result.interval = cal->interval();
	result.guiderate = cal->guiderate();
	for (int i = 0; i < 6; i++) {
		result.coefficients.push_back(cal->a[i]);
	}
	result.complete = cal->complete();
	result.flipped = cal->flipped();
	result.meridianFlipped = cal->meridian_flipped();
	result.det  = cal->det();
	result.quality = cal->quality();

	// copy the calibration points
	for (unsigned int i = 0; i < cal->size(); i++) {
		result.points.push_back(convert((*cal)[i]));
	}

	return result;
}

astro::guiding::CalibrationPtr	convert(const Calibration& cal) {
	astro::guiding::GuiderName	guidername
		= convertnameGuiderName(cal.instrument);
	astro::guiding::ControlDeviceName	cdname(guidername,
		convertcontroltype(cal.type));
	astro::guiding::BasicCalibration	*result = NULL;
	switch (cal.type) {
	case ControlGuidePort:
		result = new astro::guiding::GuiderCalibration(cdname);
		break;
	case ControlAdaptiveOptics:
		result = new astro::guiding::AdaptiveOpticsCalibration(cdname);
		break;
	}
	result->calibrationid(cal.id);
	result->when(converttime(cal.timeago));
	result->east((cal.east == 1) ? true : false);
	result->declination(astro::Angle(cal.declination,
		astro::Angle::Degrees));
	result->calibrationtype(convertcontroltype(cal.type));

	for (int i = 0; i < 6; i++) {
		result->a[i] = cal.coefficients[i];
	}
	result->complete(cal.complete);
	result->flipped(cal.flipped);
	result->meridian_flipped(cal.meridianFlipped);
	result->masPerPixel(cal.masPerPixel);
	result->focallength(cal.focallength);
	result->interval(cal.interval);
	result->guiderate(cal.guiderate);

	for (unsigned int i = 0; i < cal.points.size(); i++) {
		result->push_back(convert(cal.points[i]));
	}
	return astro::guiding::CalibrationPtr(result);
}

struct TrackingSummary	convert(const astro::guiding::TrackingSummary& summary) {
	struct TrackingSummary	result;
	result.instrument = summary.descriptor.instrument();
	result.since = converttime(summary.starttime);
	result.guideportcalid = summary.guideportcalid;
	result.adaptiveopticscalid = summary.adaptiveopticscalid;
	result.trackid = summary.trackingid;
	result.points = summary.count();
	result.lastoffset = convert(summary.lastoffset);
	result.averageoffset = convert(summary.averageoffset());
	result.variance = convert(summary.variance());
	return result;
}

astro::guiding::TrackingSummary	convert(const struct TrackingSummary& summary) {
	astro::guiding::TrackingSummary	result(summary.instrument);
	result.starttime = converttime(summary.since);
	result.trackingid = summary.trackid;
	result.guideportcalid = summary.guideportcalid;
	result.adaptiveopticscalid = summary.adaptiveopticscalid;
	result.count(summary.points);
	result.lastoffset = convert(summary.lastoffset);
	result.average(convert(summary.averageoffset));
	result.variance(convert(summary.variance));
	return result;
}

std::string	calibrationtype2string(ControlType caltype) {
	switch (caltype) {
	case ControlGuidePort:
		return std::string("GP");
	case ControlAdaptiveOptics:
		return std::string("AO");
	}
	std::string	cause = astro::stringprintf("unknown cal type %d",
				caltype);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
	throw std::runtime_error(cause);
}

ControlType	string2calibrationtype(const std::string& caltype) {
	if ((caltype == "GP") || (caltype == "GuidePort")) {
		return ControlGuidePort;
	}
	if ((caltype == "AO") || (caltype == "AdaptiveOptics")) {
		return ControlAdaptiveOptics;
	}
	std::string	cause = astro::stringprintf("unknown cal type '%s'",
		caltype.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
	throw std::runtime_error(cause);
}

CalibrationImageProgress        convert(const astro::camera::CalibrationImageProgress prog) {
	CalibrationImageProgress	result;
	result.imageno = prog.imageno;
	result.imagecount = prog.imagecount;
	return result;
}

astro::camera::CalibrationImageProgress        convert(const CalibrationImageProgress prog) {
	astro::camera::CalibrationImageProgress	result;
	result.imageno = prog.imageno;
	result.imagecount = prog.imagecount;
	return result;
}

// methods related to backlash characterization process
BacklashPoint   convert(const astro::guiding::BacklashPoint& point) {
	BacklashPoint	p;
	p.seqno = point.id;
	p.time = point.time;
	p.xoffset = point.xoffset;
	p.yoffset = point.yoffset;
	return p;
}

astro::guiding::BacklashPoint   convert(const BacklashPoint& point) {
	astro::guiding::BacklashPoint	p;
	p.id = point.seqno;
	p.time = point.time;
	p.xoffset = point.xoffset;
	p.yoffset = point.yoffset;
	return p;
}

BacklashPoints  convert(const astro::guiding::BacklashPoints& points) {
	BacklashPoints	ps;
	std::for_each(points.begin(), points.end(),
		[&ps](const astro::guiding::BacklashPoint& p) mutable {
			ps.push_back(convert(p));
		}
	);
	return ps;
}

astro::guiding::BacklashPoints  convert(const BacklashPoints& points) {
	astro::guiding::BacklashPoints	ps;
	std::for_each(points.begin(), points.end(),
		[&ps](const BacklashPoint& p) mutable {
			ps.push_back(convert(p));
		}
	);
	return ps;
}

BacklashResult  convert(const astro::guiding::BacklashResult& result) {
	BacklashResult	r;
	r.direction = (result.direction == astro::guiding::backlash_dec)
			? BacklashDEC : BacklashRA;
	r.lastpoints = result.lastpoints;
	r.interval = result.interval;
	r.x = result.x;
	r.y = result.y;
	r.longitudinal = result.longitudinal;
	r.lateral = result.lateral;
	r.forward = result.forward;
	r.backward = result.backward;
	r.f = result.f;
	r.b = result.b;
	r.offset = result.offset;
	r.drift = result.drift;
	return r;
}

astro::guiding::BacklashResult  convert(const BacklashResult& result) {
	astro::guiding::BacklashResult	r;
	r.direction = (result.direction == BacklashDEC)
			? astro::guiding::backlash_dec
			: astro::guiding::backlash_ra;
	r.lastpoints = result.lastpoints;
	r.interval = result.interval;
	r.x = result.x;
	r.y = result.y;
	r.longitudinal = result.longitudinal;
	r.lateral = result.lateral;
	r.forward = result.forward;
	r.backward = result.backward;
	r.f = result.f;
	r.b = result.b;
	r.offset = result.offset;
	r.drift = result.drift;
	return r;
}

BacklashData    convert(const astro::guiding::BacklashData& data) {
	BacklashData	d;
	d.points = convert(data.points);
	d.result = convert(data.result);
	return d;
}

astro::guiding::BacklashData    convert(const BacklashData& data) {
	astro::guiding::BacklashData	d;
	d.points = convert(data.points);
	d.result = convert(data.result);
	return d;
}

astro::guiding::backlash_t	convert(BacklashDirection dir) {
	switch (dir) {
	case BacklashDEC:	return astro::guiding::backlash_dec;
	case BacklashRA:	return astro::guiding::backlash_ra;
	}
	throw std::runtime_error("unknown backlash direction");
}

BacklashDirection	convert(astro::guiding::backlash_t dir) {
	switch (dir) {
	case astro::guiding::backlash_dec: return BacklashDEC;
	case astro::guiding::backlash_ra: return BacklashRA;
	}
	throw std::runtime_error("unknown backlash direction");
}

} // namespace snowstar
