/*
 * GuiderIconversions.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuiderFactoryI.h>
#include <GuiderI.h>
#include <TypesI.h>
#include <AstroGuiding.h>

namespace snowstar {

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
	result.timeago = trackingpoint.t;
	result.trackingoffset = convert(trackingpoint.trackingoffset);
	result.activation = convert(trackingpoint.correction);
	return result;
}

CalibrationPoint        convert(const astro::guiding::CalibrationPoint& cp) {
	CalibrationPoint	result;
	result.t = cp.t;
	result.offset = convert(cp.offset);
	result.star = convert(cp.star);
	return result;
}

}
