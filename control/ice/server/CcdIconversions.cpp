/*
 * CcdIconversions.cpp -- conversions used in the CcdI interface
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CcdI.h>
#include <CoolerI.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Communicator.h>
#include <AstroDebug.h>

namespace snowstar {

CcdInfo	CcdI::convert(const astro::camera::CcdInfo& info) {
	CcdInfo	result;
	result.name = info.name();
	result.id = info.getId();
	result.size.width = info.size().width();
	result.size.height = info.size().height();
	result.shutter = info.shutter();
	result.pixelheight = info.pixelheight();
	result.pixelwidth = info.pixelwidth();
	astro::camera::BinningSet::const_iterator	b;
	for (b = info.modes().begin(); b != info.modes().end(); b++) {
		BinningMode	mode;
		mode.x = b->getX();
		mode.y = b->getY();
		result.binningmodes.push_back(mode);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccdinfo has %d binning modes",
		result.binningmodes.size());
	return result;
}

Exposure	CcdI::convert(const astro::camera::Exposure& exp) {
	Exposure	exposure;
	exposure.frame.origin.x = exp.frame.origin().x();
	exposure.frame.origin.y = exp.frame.origin().y();
	exposure.frame.size.width = exp.frame.size().width();
	exposure.frame.size.height = exp.frame.size().height();
	exposure.exposuretime = exp.exposuretime;
	exposure.gain = exp.gain;
	exposure.limit = exp.limit;
	exposure.shutter = convert(exp.shutter);
	exposure.mode.x = exp.mode.getX();
	exposure.mode.y = exp.mode.getY();
	return exposure;
}

astro::camera::Exposure	CcdI::convert(const Exposure& exposure) {
	astro::camera::Exposure	exp;
	exp.frame = astro::image::ImageRectangle(
		astro::image::ImagePoint(exposure.frame.origin.x,
			exposure.frame.origin.y),
		astro::image::ImageSize(exposure.frame.size.width,
			exposure.frame.size.height));
	exp.exposuretime = exposure.exposuretime;
	exp.gain = exposure.gain;
	exp.limit = exposure.limit;
	exp.shutter = convert(exposure.shutter);
	exp.mode.setX(exposure.mode.x);
	exp.mode.setY(exposure.mode.y);
	return exp;
}

ExposureState	CcdI::convert(const astro::camera::Exposure::State& s) {
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

astro::camera::Exposure::State	CcdI::convert(const ExposureState& s) {
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

ShutterState	CcdI::convert(const astro::camera::shutter_state& s) {
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

astro::camera::shutter_state	CcdI::convert(const ShutterState& s) {
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

} // namespace snowstar
