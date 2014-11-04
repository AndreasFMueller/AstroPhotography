/*
 * GuiderConversions.cpp -- conversions between ice and astro
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <IceConversions.h>
#include <includes.h>
#include <AstroIO.h>
#include <AstroUtils.h>

namespace snowstar {

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
	}
	throw std::runtime_error("unknown guider state");
}

GuiderState	string2guiderstate(const std::string& s) {
	return convert(astro::guiding::Guide::string2state(s));
}

std::string	guiderstate2string(GuiderState state) {
	return astro::guiding::Guide::state2string(convert(state));
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

astro::guiding::TrackingHistory	convert(const TrackingHistory& history) {
	astro::guiding::TrackingHistory	result;
	result.camera = history.guider.cameraname;
	result.ccdid = history.guider.ccdid;
	result.guiderport = history.guider.guiderportname;
	result.whenstarted = converttime(history.timeago);
	for (auto ptr = history.points.begin(); ptr != history.points.end();
		ptr++) {
		result.points.push_back(convert(*ptr));
	}
	return result;
}

TrackingHistory	convert(const astro::guiding::TrackingHistory& history) {
	TrackingHistory	result;
	result.guider.cameraname = history.camera;
	result.guider.ccdid = history.ccdid;
	result.guider.guiderportname = history.guiderport;
	result.timeago = converttime(history.whenstarted);
	for (auto ptr = history.points.begin(); ptr != history.points.end();
		ptr++) {
		result.points.push_back(convert(*ptr));
	}
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

} // namespace snowstar
