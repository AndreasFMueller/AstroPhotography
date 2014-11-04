/*
 * CcdConversions.cpp -- conversions between ice and astro
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <IceConversions.h>
#include <includes.h>
#include <AstroIO.h>
#include <AstroUtils.h>

namespace snowstar {

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
	throw std::runtime_error("unknown exposure state");
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
	throw std::runtime_error("unknown exposure state");
}

ShutterState	string2shutterstate(const std::string& s) {
	return convert(astro::camera::Shutter::string2state(s));
}

std::string	state2string(ShutterState state) {
	return astro::camera::Shutter::state2string(convert(state));
}

ShutterState	convert(const astro::camera::Shutter::state& s) {
	ShutterState	result = snowstar::ShOPEN;
	switch (s) {
	case astro::camera::Shutter::OPEN:
		result = snowstar::ShOPEN;
		break;
	case astro::camera::Shutter::CLOSED:
		result = snowstar::ShCLOSED;
		break;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s",
		astro::camera::Shutter::state2string(s).c_str(),
		state2string(result).c_str());
	return result;
}

astro::camera::Shutter::state	convert(const ShutterState& s) {
	astro::camera::Shutter::state	result = astro::camera::Shutter::OPEN;
	switch (s) {
	case snowstar::ShOPEN:
		result = astro::camera::Shutter::OPEN;
		break;
	case snowstar::ShCLOSED:
		result = astro::camera::Shutter::CLOSED;
		break;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s",
		(s == ShOPEN) ? "open" : "closed",
		astro::camera::Shutter::state2string(result).c_str());
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

} // namespace snowstar
