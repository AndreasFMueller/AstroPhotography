/*
 * Tracker.cpp -- classes implementing star trackers
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>

#include <AstroGuiding.h>
#include <AstroUtils.h>
#include <sstream>
#include <AstroDebug.h>

using namespace astro::image;
using namespace astro::image::transform;
using namespace astro::adapter;

namespace astro {
namespace guiding {

#if 0
//////////////////////////////////////////////////////////////////////
// Implementation of the Phase Tracker
//////////////////////////////////////////////////////////////////////

PhaseTracker::PhaseTracker() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing a phase tracker");
	_image = NULL;
}

Point	PhaseTracker::operator()(ImagePtr newimage) {
	if (!_imageptr) {
		ConstImageAdapter<double>	*a = adapter(newimage);
		refresh(*a);
		delete a;
		return Point(0,0);
	}
	ConstImageAdapter<double>	*a = adapter(newimage);
	PhaseCorrelator	pc;
	Point	result = correlate(*a, pc);
	delete a;
	return result;
}

std::string	PhaseTracker::toString() const {
	std::string	info = stringprintf("PhaseTracker on %s image",
			(_image) ? _image->size().toString().c_str()
				: "(undefined)");
	return info;
}

//////////////////////////////////////////////////////////////////////
// Implementation of the Differential Phase Tracker
//////////////////////////////////////////////////////////////////////

DifferentialPhaseTracker::DifferentialPhaseTracker() {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"constructing a differential phase tracker");
	_image = NULL;
}

Point	DifferentialPhaseTracker::operator()(ImagePtr newimage) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting offset from %s image",
		newimage->size().toString().c_str());
	if (!_imageptr) {
		ConstImageAdapter<double>	*a = adapter(newimage);
		refresh(*a);
		delete a;
		return Point(0,0);
	}
	ConstImageAdapter<double>	*a = adapter(newimage);
	DerivativePhaseCorrelator	pc;
	Point	result = correlate(*a, pc);
	delete a;
	return result;
}

std::string	DifferentialPhaseTracker::toString() const {
	std::string	info
		= stringprintf("DifferentialPhaseTracker on %s image",
			(_image) ? _image->size().toString().c_str()
				: "(undefined)");
	return info;
}

#endif

} // namespace guiding
} // namespace astro
