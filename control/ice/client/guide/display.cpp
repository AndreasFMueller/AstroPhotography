/*
 * display.cpp -- display classes for calibration and tracking 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hohschule Rapperswil
 */
#include "display.h"
#include <AstroFormat.h>
#include <AstroUtils.h>
#include <IceConversions.h>
#include <cmath>

namespace snowstar {
namespace app {
namespace snowguide {

//////////////////////////////////////////////////////////////////////
// calibration point display class implementation
//////////////////////////////////////////////////////////////////////

CalibrationPoint_display::CalibrationPoint_display(std::ostream& out)
	: _out(out) {
}

void	CalibrationPoint_display::operator()(const CalibrationPoint& calpoint) {
	_out << "         ";
	_out << astro::stringprintf("%.1f: ", calpoint.t);
	_out << astro::stringprintf("(%f,%f) -> (%f,%f)",
			calpoint.offset.x, calpoint.offset.y,
			calpoint.star.x, calpoint.star.y);
	_out << std::endl;
}

//////////////////////////////////////////////////////////////////////
// tracking point display class implementation
//////////////////////////////////////////////////////////////////////

Calibration_display::Calibration_display(std::ostream& out) : _out(out) {
	_verbose = false;
}

void	Calibration_display::operator()(const Calibration& cal) {
	// id and timestamp
	_out << astro::stringprintf("%4d: ", cal.id);
	_out << astro::timeformat("%Y-%m-%d %H:%M, ",
		converttime(cal.timeago));
	_out << cal.points.size() << " points, ";
	_out << astro::stringprintf("quality=%.1f%%, ", 100 * cal.quality);
	switch (cal.controltype) {
	case CalibrationTypeGuiderPort:
		_out << "GP, ";
		_out << astro::stringprintf("%.3f mas/Pixel", cal.masPerPixel);
		break;
	case CalibrationTypeAdaptiveOptics:
		_out << "AO";
		break;
	}
	_out << std::endl;

	// calibration coefficients
	_out << std::string("     ");
	for (int k = 0; k < 3; k++) {
		_out << astro::stringprintf("%12.8f", cal.coefficients[k]);
	}
	_out << std::endl << std::string("     ");
	for (int k = 3; k < 6; k++) {
		_out << astro::stringprintf("%12.8f", cal.coefficients[k]);
	}
	_out << std::endl;

	// calibration points if verbose
	if (_verbose) {
		std::for_each(cal.points.begin(), cal.points.end(),
			CalibrationPoint_display(_out));
	}
}

//////////////////////////////////////////////////////////////////////
// tracking point display class implementation
//////////////////////////////////////////////////////////////////////

TrackingPoint_display::TrackingPoint_display(std::ostream& out,
	double starttime) : _out(out), _starttime(starttime) {
	_csv = false;
	counter = 0;
}

void	TrackingPoint_display::operator()(const TrackingPoint& point) {
	if (_csv) {
		_out << astro::stringprintf("%6d,", ++counter);
		_out << astro::stringprintf("%8.1f,",
			_starttime - point.timeago);
		_out << astro::stringprintf("%10.4f,%10.4f,%10.4f,%10.4f",
				point.trackingoffset.x, point.trackingoffset.y,
				point.activation.x, point.activation.y);
		if (0.1 < masperpixel()) {
			double	p = hypot(point.trackingoffset.x,
				point.trackingoffset.y) * masperpixel();
			_out << astro::stringprintf(",%8.0f", p);
		}
	} else {
		_out << astro::stringprintf("[%04d] ", ++counter);
		_out << astro::timeformat("%Y-%m-%d %H:%M:%S",
			converttime(point.timeago));
		_out << astro::stringprintf(".%03.0f ",
			1000 * (point.timeago - trunc(point.timeago)));
		_out << astro::stringprintf("(%6.2f,%6.2f) -> (%6.2f,%6.2f)",
				point.trackingoffset.x, point.trackingoffset.y,
				point.activation.x, point.activation.y);
	}
	_out << std::endl;
}

} // namespace snowguide
} // namespace app
} // namespace snowstar
