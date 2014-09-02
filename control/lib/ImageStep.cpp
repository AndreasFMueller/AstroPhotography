/*
 * ImageStep.cpp -- processing steps that represent image
 *
 * This was historically the first piece of the project that used lambdas
 * in an essential way
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperwil
 */
#include <AstroProcess.h>
#include <algorithm>
#include <includes.h>
#include <AstroDebug.h>

using namespace astro::adapter;

namespace astro {
namespace process {

//////////////////////////////////////////////////////////////////////
// Construction and Destruction
//////////////////////////////////////////////////////////////////////
/**
 * \brief Create a new processing step
 */
ImageStep::ImageStep() : _preview(NULL), _out(NULL) {
	_status = idle;
}

/**
 * \brief Destroy the processing step
 */
ImageStep::~ImageStep() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroying an %s", type_name().c_str());
}

//////////////////////////////////////////////////////////////////////
// Preview access
//////////////////////////////////////////////////////////////////////
PreviewMonochromeAdapter	ImageStep::monochrome_preview() {
	return PreviewMonochromeAdapter(preview());
}

PreviewColorAdapter	ImageStep::color_preview() {
	return PreviewColorAdapter(preview());
}

//////////////////////////////////////////////////////////////////////
// Access to output images
//////////////////////////////////////////////////////////////////////
const ConstImageAdapter<double>&	ImageStep::out() const {
	if (NULL == _out) {
		throw std::runtime_error("no output available");
	}
	return *_out;
}

bool	ImageStep::hasColor() const {
	return false;
}

const ConstImageAdapter<RGB<double> >&	ImageStep::out_color() const {
	throw std::runtime_error("not implemented");
}

} // namespace process
} // namespace astro
