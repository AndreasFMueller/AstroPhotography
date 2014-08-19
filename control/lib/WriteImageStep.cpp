/*
 * WriteImageStep.cpp -- processing step to write an image
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroIO.h>
#include <AstroDebug.h>

using namespace astro::io;
using namespace astro::adapter;

namespace astro {
namespace process {

/**
 * \brief Construct an image writer
 */
WriteImage::WriteImage(const std::string& filename, bool precious)
	: _filename(filename), _precious(precious) {
}

/**
 * \brief Get the input step
 */
ImageStep	*WriteImage::input() const {
	steps::const_iterator	i
		= std::find_if(precursors().begin(), precursors().end(),
			[](ProcessingStep *step) {
				ImageStep	*image
					= dynamic_cast<ImageStep *>(step);
				return (NULL != image);
			}
		);
	if (i == precursors().end()) {
		std::runtime_error("no precursors set");
	}
	return dynamic_cast<ImageStep *>(*i);
}

/**
 * \brief Do the actual work of writing a file
 */
ProcessingStep::state	WriteImage::do_work() {
	try {
		//
		// turn the precessor's output into an image
		Image<double>	image(input()->out());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got %s image",
			image.size().toString().c_str());

		// write the image
		FITSoutfile<double>	outfile(_filename);
		outfile.setPrecious(_precious);
		outfile.write(image);

		// job done
		return ProcessingStep::complete;
	} catch (std::exception& x) {
		return ProcessingStep::idle;
	}
}

/**
 * \brief Previewing the file write 
 *
 * Previewing the file write process doesn't make much sense, but if you
 * insist, we can give you the preview of the input, which should be
 * identical.
 */
PreviewAdapterPtr	WriteImage::preview() const {
	return input()->preview();
}

/**
 * \brief Output of the File writer
 *
 * Writing a file doesn't really produce an image output, so we just
 * forward the output of the input
 */
const ConstImageAdapter<double>&        WriteImage::out() const {
	return input()->out();
}


} // namespace process
} // namespace astro
