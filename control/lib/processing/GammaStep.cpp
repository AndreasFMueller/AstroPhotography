/*
 * GammaStep.cpp -- implementation of the color step
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>

namespace astro {
namespace process {

/**
 * \brief Construct a new GammaStep
 */
GammaStep::GammaStep(NodePaths& parent) : ImageStep(parent) {
}

ProcessingStep::state	GammaStep::do_work() {
	switch (status()) {
	case ProcessingStep::needswork:
	case ProcessingStep::complete:
		return ProcessingStep::complete;
	default:
		return ProcessingStep::idle;
	}
}

std::string	GammaStep::what() const {
	return std::string("Gamma correction");
}

ImagePtr	GammaStep::image() {
	ImagePtr	precursor = precursorimage();
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"applying gamma correction min=%.1f, max=%.1f, gamma=%.1f",
		_minimum, _maximum, _gamma);
	return adapter::gammatransform(precursor, *this);
}

} // namespace process
} // namespace astro
