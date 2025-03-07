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
BinningMode	convert(const astro::image::Binning& binning) {
	BinningMode	result;
	result.x = binning.x();
	result.y = binning.y();
	return result;
}

astro::image::Binning	convert(const BinningMode& mode) {
	return astro::image::Binning(mode.x, mode.y);
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
	result.minexposuretime = info.minexposuretime();
	result.maxexposuretime = info.maxexposuretime();
	return result;
}

astro::camera::CcdInfo	convert(const CcdInfo& info) {
	astro::camera::CcdInfo	result(info.name, convert(info.size), info.id);
	result.shutter(info.shutter);
	result.pixelwidth(info.pixelwidth);
	result.pixelheight(info.pixelheight);
	result.addModes(convert(info.binningmodes));
	result.minexposuretime(info.minexposuretime);
	result.maxexposuretime(info.maxexposuretime);
	return result;
}

Exposure	convert(const astro::camera::Exposure& exp) {
	Exposure	exposure;
	exposure.frame = convert(exp.frame());
	exposure.exposuretime = exp.exposuretime();
	exposure.gain = exp.gain();
	exposure.limit = exp.limit();
	exposure.shutter = convert(exp.shutter());
	exposure.purpose = convert(exp.purpose());
	exposure.mode = convert(exp.mode());
	exposure.quality = convert(exp.quality());
	return exposure;
}

astro::camera::Exposure	convert(const Exposure& exposure) {
	astro::camera::Exposure	exp;
	exp.frame(convert(exposure.frame));
	exp.exposuretime(exposure.exposuretime);
	exp.gain(exposure.gain);
	exp.limit(exposure.limit);
	exp.shutter(convert(exposure.shutter));
	exp.purpose(convert(exposure.purpose));
	exp.mode(convert(exposure.mode));
	exp.quality(convert(exposure.quality));
	return exp;
}

ExposureState	convert(const astro::camera::CcdState::State& s) {
	switch (s) {
	case astro::camera::CcdState::idle:
		return IDLE;
	case astro::camera::CcdState::exposing:
		return EXPOSING;
	case astro::camera::CcdState::cancelling:
		return CANCELLING;
	case astro::camera::CcdState::streaming:
		return STREAMING;
	case astro::camera::CcdState::exposed:
		return EXPOSED;
	}
	throw std::runtime_error("unknown exposure state");
}

astro::camera::CcdState::State	convert(const ExposureState& s) {
	switch (s) {
	case IDLE:
		return astro::camera::CcdState::idle;
	case EXPOSING:
		return astro::camera::CcdState::exposing;
	case CANCELLING:
		return astro::camera::CcdState::cancelling;
	case STREAMING:
		return astro::camera::CcdState::streaming;
	case EXPOSED:
		return astro::camera::CcdState::exposed;
	case BROKEN:
		// there is no corresponding state, so we just return idle
		return astro::camera::CcdState::idle;
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
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s",
	//	astro::camera::Shutter::state2string(s).c_str(),
	//	state2string(result).c_str());
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
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s",
	//	(s == ShOPEN) ? "open" : "closed",
	//	astro::camera::Shutter::state2string(result).c_str());
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
	case astro::camera::Exposure::bias:
		return snowstar::ExBIAS;
	case astro::camera::Exposure::test:
		return snowstar::ExTEST;
	case astro::camera::Exposure::guide:
		return snowstar::ExGUIDE;
	case astro::camera::Exposure::focus:
		return snowstar::ExFOCUS;
	case astro::camera::Exposure::flood:
		return snowstar::ExFLOOD;
	case astro::camera::Exposure::preview:
		return snowstar::ExPREVIEW;
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
	case snowstar::ExBIAS:
		return astro::camera::Exposure::bias;
	case snowstar::ExTEST:
		return astro::camera::Exposure::test;
	case snowstar::ExGUIDE:
		return astro::camera::Exposure::guide;
	case snowstar::ExFOCUS:
		return astro::camera::Exposure::focus;
	case snowstar::ExFLOOD:
		return astro::camera::Exposure::flood;
	case snowstar::ExPREVIEW:
		return astro::camera::Exposure::preview;
	}
	throw std::runtime_error("unknown exposure purpose");
}

astro::camera::Exposure::quality_t	convert(const ExposureQuality& quality) {
	switch (quality) {
	case ExQualityHIGH:
		return astro::camera::Exposure::high;
	case ExQualityFAST:
		return astro::camera::Exposure::fast;
	}
	throw std::runtime_error("unknown quality");
}

ExposureQuality	convert(const astro::camera::Exposure::quality_t& quality) {
	switch (quality) {
	case astro::camera::Exposure::high:
		return ExQualityHIGH;
	case astro::camera::Exposure::fast:
		return ExQualityFAST;
	}
	throw std::runtime_error("unknown quality");
}

std::string	quality2string(const ExposureQuality& quality) {
	switch (quality) {
	case ExQualityHIGH:
		return std::string("high");
	case ExQualityFAST:
		return std::string("fast");
	}
	throw std::runtime_error("unknown quality");
}

ExposureQuality	string2quality(const std::string& s) {
	if (s == "high") {
		return ExQualityHIGH;
	}
	if (s == "fast") {
		return ExQualityFAST;
	}
	throw std::runtime_error("unknown quality");
}

std::pair<float, float>	convert(const Interval& interval) {
	return std::make_pair(interval.min, interval.max);
}

Interval	convert(const std::pair<float, float>& interval) {
	Interval	result;
	result.min = interval.first;
	result.max = interval.second;
	return result;
}

ImageQueueEntryPtr	convert(const astro::camera::ImageQueueEntry e) {
	ImageQueueEntryPtr	result(new ImageQueueEntry());
	result->exposure0 = convert(e.exposure);
	result->imagedata = convertfile(e.image);
	return result;
}

astro::camera::ImageQueueEntry	convert(ImageQueueEntryPtr e) {
	astro::camera::ImageQueueEntry	result(convert(e->exposure0),
		convertfile(e->imagedata));
	return result;
}

} // namespace snowstar
