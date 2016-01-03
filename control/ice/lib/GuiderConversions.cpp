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

#if 0
static std::string	index2name(const std::string& instrumentname, int index,
			astro::discover::InstrumentComponentKey::Type type) {
	// first try to look up the device in the instrument database
	try {
		// get an instrument for the instrument name
		astro::discover::InstrumentPtr  instrument
			= astro::discover::InstrumentBackend::get(
				instrumentname);
		return instrument->get(type, index).deviceurl();
	} catch (...) {
	}
	// build an unknown name from the index
	std::string	devicename;
	switch (type) {
	case astro::discover::InstrumentComponentKey::AdaptiveOptics:
		devicename = "adaptiveoptics";
		break;
	case astro::discover::InstrumentComponentKey::Camera:
		devicename = "camera";
		break;
	case astro::discover::InstrumentComponentKey::CCD:
	case astro::discover::InstrumentComponentKey::GuiderCCD:
		devicename = "ccd";
		break;
	case astro::discover::InstrumentComponentKey::Cooler:
		devicename = "camera";
		break;
	case astro::discover::InstrumentComponentKey::GuiderPort:
		devicename = "guiderport";
		break;
	case astro::discover::InstrumentComponentKey::FilterWheel:
		devicename = "filterwheel";
		break;
	case astro::discover::InstrumentComponentKey::Focuser:
		devicename = "focuser";
		break;
	case astro::discover::InstrumentComponentKey::Mount:
		devicename = "mount";
		break;
	default:
		throw std::runtime_error("unknown device type");
		break;
	}
	std::string deviceurl = astro::stringprintf("%s:unknown/%d",
		devicename.c_str(), index);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown device: %s", deviceurl.c_str());
	return deviceurl;
}

static int	name2index(const std::string& instrumentname,
			const std::string& deviceurl,
			astro::discover::InstrumentComponentKey::Type type) {
	// handle the case where the device url comes from the "unknown" module
	astro::DeviceName	devname(deviceurl);
	if (devname.modulename() == "unknown") {
		try {
			return std::stoi(devname.unitname());
		} catch (...) {
			return 0;
		}
	}
	// handle all other device names, try to convert them into an index
	try {
		astro::discover::InstrumentPtr  instrument
			= astro::discover::InstrumentBackend::get(instrumentname);
		int	index = instrument->indexOf(type, deviceurl);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s has index %d in %s",
			deviceurl.c_str(), index, instrumentname.c_str());
	} catch (...) { }
	// return 0 if this is unknown
	return 0;
}
#endif

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

/*
int     instrumentName2index(const std::string& instrument,
                const InstrumentComponentType type, const std::string& deviceurl);
std::string     instrumentIndex2name(const std::string& instrument,
                const InstrumentComponentType type, int index);
*/

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

} // namespace snowstar
