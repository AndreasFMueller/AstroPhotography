/*
 * ColorclampStep.cpp -- implementation of the Colorclamp step
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <sstream>
#include <AstroAdapter.h>

namespace astro {
namespace process {

/**
 * \brief Construct a new ColorclampStep
 */
ColorclampStep::ColorclampStep(NodePaths& parent) : ImageStep(parent) {
	_minimum = 0;
	_maximum = 255;
}

ProcessingStep::state	ColorclampStep::do_work() {
	switch (status()) {
	case ProcessingStep::needswork:
	case ProcessingStep::complete:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "colorclamp is complete");
		return ProcessingStep::complete;
	default:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "colorclamp is idle");
		return ProcessingStep::idle;
	}
}

std::string	ColorclampStep::what() const {
	std::ostringstream	out;
	out << "colorclamp: ";
	if (minimum() >= 0) {
		out << "minimum = " << minimum() << " ";
	}
	if (maximum() >= 0) {
		out << "maximum = " << maximum() << " ";
	}
	return out.str();
}

ImagePtr	ColorclampStep::image() {
	ImagePtr	inimageptr = precursorimage();
	Image<RGB<float> >      *inimage
		= dynamic_cast<Image<RGB<float> >*>(&*inimageptr);
	if (NULL == inimage) {
		throw std::runtime_error("unknown image format");
	}
	adapter::ColorLuminanceAdapter<float>     clamp(*inimage, minimum(),
							maximum());
	Image<RGB<float> >      *outimage = new Image<RGB<float> >(clamp);
	ImagePtr        outimageptr(outimage);
	return outimageptr;
}

} // namespace process
} // namespace astro
