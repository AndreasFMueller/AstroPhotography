/*
 * ColorStep.cpp -- implementation of the color step
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>

namespace astro {
namespace process {

/**
 * \brief Construct a new ColorStep
 */
ColorStep::ColorStep(NodePaths& parent) : ImageStep(parent) {
}

ProcessingStep::state	ColorStep::do_work() {
	switch (status()) {
	case ProcessingStep::needswork:
	case ProcessingStep::complete:
		return ProcessingStep::complete;
	default:
		return ProcessingStep::idle;
	}
}

std::string	ColorStep::what() const {
	return std::string("Color correction");
}

ImagePtr	ColorStep::image() {
	ImagePtr	precursor = precursorimage();
	return adapter::colortransform(precursor, *this);
}

} // namespace process
} // namespace astro
