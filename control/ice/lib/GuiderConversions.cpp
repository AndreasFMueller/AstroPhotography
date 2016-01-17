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
	result.instrumentname = gd.instrument();
	result.ccdIndex = instrumentName2index(gd.instrument(),
				InstrumentGuiderCCD, gd.ccd());
	result.guiderportIndex = instrumentName2index(gd.instrument(),
				InstrumentGuiderPort, gd.guiderport());
	return result;
}

astro::guiding::GuiderDescriptor        convert(const GuiderDescriptor& gd) {
	// convert ccd index into a name
	std::string	ccdname = instrumentIndex2name(gd.instrumentname,
				InstrumentGuiderCCD, gd.ccdIndex);
	// convert guiderport index into a name
	std::string	guiderportname = instrumentIndex2name(gd.instrumentname,
				InstrumentGuiderPort, gd.ccdIndex);
	astro::guiding::GuiderDescriptor	result(gd.instrumentname,
					ccdname, guiderportname);
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
	result.instrument = history.guider.instrumentname;
	result.ccd = instrumentIndex2name(result.instrument,
		InstrumentGuiderCCD, history.guider.ccdIndex);
	result.guiderport = instrumentIndex2name(result.instrument,
		InstrumentGuiderPort,
		history.guider.guiderportIndex);
	result.whenstarted = converttime(history.timeago);
	for (auto ptr = history.points.begin(); ptr != history.points.end();
		ptr++) {
		result.points.push_back(convert(*ptr));
	}
	return result;
}

TrackingHistory	convert(const astro::guiding::TrackingHistory& history) {
	TrackingHistory	result;
	result.guider.instrumentname = history.instrument;
	result.guider.ccdIndex = instrumentName2index(history.instrument,
		InstrumentGuiderCCD, history.ccd);
	result.guider.guiderportIndex = instrumentName2index(history.instrument,
		InstrumentGuiderPort, history.guiderport);
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

struct TrackingSummary	convert(const astro::guiding::TrackingSummary& summary) {
	struct TrackingSummary	result;
	result.guider.instrumentname = summary.descriptor.instrument();
	result.guider.ccdIndex = instrumentName2index(
		summary.descriptor.instrument(), InstrumentGuiderCCD,
		summary.descriptor.ccd());
	result.guider.guiderportIndex = instrumentName2index(
		summary.descriptor.instrument(), InstrumentGuiderPort,
		summary.descriptor.guiderport());
	result.since = converttime(summary.starttime);
	result.lastoffset = convert(summary.lastoffset);
	result.averageoffset = convert(summary.averageoffset());
	result.variance = convert(summary.variance());
	return result;
}

astro::guiding::TrackingSummary	convert(const struct TrackingSummary& summary) {
	std::string	ccdname = instrumentIndex2name(
		summary.guider.instrumentname, InstrumentGuiderCCD,
		summary.guider.ccdIndex);
	std::string	guiderportname = instrumentIndex2name(
		summary.guider.instrumentname, InstrumentGuiderPort,
		summary.guider.guiderportIndex);
	astro::guiding::TrackingSummary	result(summary.guider.instrumentname,
		ccdname, guiderportname);
	result.starttime = converttime(summary.since);
	result.calibrationid = summary.calibrationid;
	result.lastoffset = convert(summary.lastoffset);
	result.average(convert(summary.averageoffset));
	result.variance(convert(summary.variance));
	return result;
}

} // namespace snowstar
