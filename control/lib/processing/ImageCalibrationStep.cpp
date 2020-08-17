/*
 * ImageCalibrationStep.cpp -- Calibration of raw images
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroCalibration.h>
#include <AstroInterpolation.h>
#include <AstroDemosaic.h>
#include <AstroOperators.h>
#include <AstroImager.h>
#include <AstroImageops.h>
#include <algorithm>
#include <sstream>

using namespace astro::image;
using namespace astro::adapter;

namespace astro {
namespace process {

/**
 * \brief Construct a new Image step
 */
ImageCalibrationStep::ImageCalibrationStep(NodePaths& parent)
	: ImageStep(parent) {
	_interpolate = true;
	_demosaic = false;
	_flip = false;
}

/**
 * \brief Perform the step
 */
ProcessingStep::state	ImageCalibrationStep::do_work() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start work in calibration");
	int	darkid = -1;
	int	flatid = -1;

	astro::camera::Imager	imager;

	// check for the dark image
	if (_dark) {
		darkid = _dark->id();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for dark image %d",
			darkid);
		ImageStep	*imagestep = dynamic_cast<ImageStep*>(&*_dark);
		if (imagestep) {
			ImagePtr	darkimage = imagestep->image();
			imager.dark(darkimage);
			imager.darksubtract(true);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "found %s dark image",
				darkimage->size().toString().c_str());
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "dark image not found");
		}
	}

	// check for the flat image
	if (_flat) {
		flatid = _flat->id();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for flat image %d",
			flatid);
		ImageStep	*imagestep = dynamic_cast<ImageStep*>(&*_flat);
		if (imagestep) {
			ImagePtr	flatimage = imagestep->image();
			imager.flat(flatimage);
			imager.flatdivide(true);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "found %s flat image",
				flatimage->size().toString().c_str());
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "flat image not found");
		}
	}

	// get the unique precursor that is not dark/flat
	steps::const_iterator	i;
	i = std::find_if(precursors().begin(), precursors().end(),
		[darkid,flatid](int id) -> bool {
			return (darkid != id) && (flatid != id);
		}
	);
	if (i == precursors().end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no precursor step");
		return ProcessingStep::failed;
	}
	ProcessingStepPtr	precursor = byid(*i);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "precursor is %s",
		precursor->name().c_str());
	ImageStep	*imagestep = dynamic_cast<ImageStep*>(&*precursor);
	if (NULL == imagestep) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no precursor image");
		return ProcessingStep::failed;
	}
	_image = astro::image::ops::duplicate(imagestep->image());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "precursor image duplicate: %s, %s",
		_image->size().toString().c_str(),
		demangle_cstr(_image->pixel_type()));

	// perform interpolation
	imager.interpolate(_interpolate);

	// perform processing
	imager(_image);

	// perform debayering
	if (_demosaic) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "demosaicing");
		_image = demosaic_bilinear(_image);
	}

	// perform flip the image
	debug(LOG_DEBUG, DEBUG_LOG, 0, "flip: %s", (_flip) ? "yes" : "no");
	if (_flip) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "flipping vertically");
		astro::image::operators::flip(_image);
	}
	if (_hflip) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "flipping horizontally");
		astro::image::operators::hflip(_image);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "image calibration complete");
	return ProcessingStep::complete;
}

ProcessingStep::state	ImageCalibrationStep::status() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "check processing status of '%s'",
		name().c_str());
	if (_image) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"processing of '%s' already complete", name().c_str());
		return ProcessingStep::complete;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking precursors");
	// if all precursors are complete, we can perform the calibration
	if (std::all_of(precursors().begin(), precursors().end(),
			[](int precursorid) -> bool {
				ProcessingStepPtr	step = byid(precursorid);
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"precursor '%s' %s",
					step->name().c_str(),
					statename(step->status()).c_str());
				return (step->status() == ProcessingStep::complete);
			}
		)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "all precursors complete");
		return ProcessingStep::needswork;
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "some precursors incomplete");
		return ProcessingStep::idle;
	}
}

std::string	ImageCalibrationStep::what() const {
	std::ostringstream	out;
	out << "calibrating: ";
	if (_dark) {
		out << "dark='" << _dark->name() << "(" << _dark->id() << "), ";
	} else {
		out << "no dark, ";
	}
	if (_flat) {
		out << "flat='" << _flat->name() << "(" << _flat->id() << "), ";
	} else {
		out << "no flat, ";
	}
	out << ((!_interpolate) ? "don't " : "") << "interpolate, ";
	out << ((!_demosaic) ? "don't " : "") << "demosaic, ";
	out << ((!_flip) ? "don't " : "") << "flip";
	return out.str();
}

} // namespace process
} // namespace astro
