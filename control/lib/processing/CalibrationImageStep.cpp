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

#if 0
//////////////////////////////////////////////////////////////////////
// the calibration image base class implementation
//////////////////////////////////////////////////////////////////////
/**
 * \brief Convert the calibration type to a string for display purposes
 */
std::string	CalibrationImageStep::caltypename(caltype t) {
	switch (t) {
	case DARK: return std::string("dark");
	case FLAT: return std::string("flat");
	}
	throw std::runtime_error("internal error, calibration types");
}

/**
 * \brief create an CalibrationImage from an image
 */
CalibrationImageStep::CalibrationImageStep(caltype t, ImagePtr image) 
                : _type(t), _image(image) {
	if (NULL != _image) {
		_out = ImageStep::outPtr(new DoubleAdapter(_image));
		_preview = PreviewAdapter::get(_image);
	}
	status(ProcessingStep::needswork);
}

/**
 * \brief Work method
 *
 * If the calibration step does not have an image, then we try to use
 * the output of the precursor
 */
ProcessingStep::state	CalibrationImageStep::do_work() {
	// if we have an image, we don't need to do anything;
	if (NULL != _image) {
		return ProcessingStep::complete;
	}

	// get the precursor
	ImageStep	*precursor = input();

	// construct an identity adapter toa ccess the precursor output
	_out = ImageStep::outPtr(new IdentityAdapter<double>(precursor->out()));

	// make the preview of the precursor available for the preview
	_preview = precursor->preview();

	// done
	return ProcessingStep::complete;
}

//////////////////////////////////////////////////////////////////////
// Calibration images read from files
//////////////////////////////////////////////////////////////////////

ProcessingStep::state	CalibrationImageFileStep::do_work() {
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

	// output
	_out = ImageStep::outPtr(new DoubleAdapter(_image));

	// that's it
	return ProcessingStep::complete;
}

//////////////////////////////////////////////////////////////////////
// access to metadata
//////////////////////////////////////////////////////////////////////
/**
 * \brief Find out whether meta data is present
 */
bool	CalibrationImageStep::hasMetadata(const std::string& name) const {
	if (NULL == _image) {
		input()->hasMetadata(name);
	}
	return _image->hasMetadata(name);
}

/**
 * \brief Access to meta data
 */
astro::image::Metavalue	CalibrationImageStep::getMetadata(const std::string& name) const {
	if (NULL == _image) {
		input()->getMetadata(name);
	}
	return _image->getMetadata(name);
}
#endif

} // namespace process
} // namespace astro

