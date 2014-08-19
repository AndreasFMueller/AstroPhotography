/*
 * CalibrationImageStep.cpp -- various types of calibration images
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroIO.h>
#include <AstroDebug.h>
#include <AstroFilterfunc.h>

using namespace astro::io;
using namespace astro::adapter;

namespace astro {
namespace process {

//////////////////////////////////////////////////////////////////////
// the calibration image base class implementation
//////////////////////////////////////////////////////////////////////
/**
 * \brief Convert the calibration type to a string for display purposes
 */
std::string	CalibrationImage::caltypename(caltype t) {
	switch (t) {
	case DARK: return std::string("dark");
	case FLAT: return std::string("flat");
	}
	throw std::runtime_error("internal error, calibration types");
}

//////////////////////////////////////////////////////////////////////
// Calibration images read from files
//////////////////////////////////////////////////////////////////////

ProcessingStep::state	CalibrationImageFile::do_work() {
	try {
		FITSin	in(_filename);
		_image = in.read();
	} catch (std::runtime_error& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot read %s: %s",
			_filename.c_str(), x.what());
		return ProcessingStep::idle;
	}

	// preview scaled in such way as to make visible the full range of
	// values
	_preview = PreviewAdapter::get(_image);
	double	min = astro::image::filter::min(_image);
	double	max = astro::image::filter::max(_image);
	if (min == max) {
		max = max + 1;
	}
	_preview->min(min);
	_preview->max(max);

	// output
	_out = ProcessingStep::outPtr(new DoubleAdapter(_image));

	// that's it
	return ProcessingStep::complete;
}


} // namespace process
} // namespace astro

