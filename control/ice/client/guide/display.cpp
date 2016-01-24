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
	_out << ((cal.complete) ? "complete, " : "incomplete, ");
	switch (cal.type) {
	case ControlGuiderPort:
		_out << "GP, ";
		_out << astro::stringprintf("%.3f mas/Pixel", cal.masPerPixel);
		break;
	case ControlAdaptiveOptics:
		_out << "AO";
		break;
	}
	_out << std::endl;
	if (_verbose) {
		_out << std::string("      guider: ");
		_out << guiderdescriptor2name(cal.guider);
		_out << std::endl;
	}

	// calibration coefficients
	_out << "      coef = [ ";
	_out << astro::stringprintf("%12.8f,", cal.coefficients[0]);
	_out << astro::stringprintf("%12.8f,", cal.coefficients[1]);
	_out << astro::stringprintf("%12.8f;", cal.coefficients[2]);
	_out << std::endl << "               ";
	_out << astro::stringprintf("%12.8f,", cal.coefficients[3]);
	_out << astro::stringprintf("%12.8f,", cal.coefficients[4]);
	_out << astro::stringprintf("%12.8f",  cal.coefficients[5]);
	_out << "  ]" << std::endl;

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
		switch (point.type) {
		case ControlGuiderPort:
			_out << ",  GP";
			break;
		case ControlAdaptiveOptics:
			_out << ",  AO";
			break;
		}
	} else {
		_out << astro::stringprintf("[%04d] ", ++counter);
		_out << astro::timeformat("%Y-%m-%d %H:%M:%S",
			converttime(point.timeago));
		_out << astro::stringprintf(".%03.0f ",
			1000 * (point.timeago - trunc(point.timeago)));
		switch (point.type) {
		case ControlGuiderPort:
			_out << "GP ";
			break;
		case ControlAdaptiveOptics:
			_out << "AO ";
			break;
		}
		_out << astro::stringprintf("(%6.2f,%6.2f) -> (%6.2f,%6.2f)",
				point.trackingoffset.x, point.trackingoffset.y,
				point.activation.x, point.activation.y);
	}
	_out << std::endl;
}


void	TrackingHistory_display::operator()(const TrackingHistory history) {
	if (_csv) {
		std::cout << "number,    time,   xoffset,   yoffset,     xcorr,     ycorr,  offset" << std::endl;
		_verbose = _csv;
	} else {
		std::cout << history.guiderunid << ": ";
		std::cout << astro::timeformat("%Y-%m-%d %H:%M",
			converttime((double)history.timeago));
		std::cout << std::endl;
	}
	if (_verbose) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "display %d tracking points",
			history.points.size());
		Calibration     cal = _guiderfactory->getCalibration(
					history.guiderportcalid);
		double  starttime = history.points.begin()->timeago;
		TrackingPoint_display   display(std::cout, starttime);
		display.csv(_csv);
		display.masperpixel(cal.masPerPixel);
		std::for_each(history.points.begin(), history.points.end(),
			display);
	}
}

} // namespace snowguide
} // namespace app
} // namespace snowstar
