/*
 * RescaleStep.cpp -- implementation of the Rescale step
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>

namespace astro {
namespace process {

/**
 * \brief Construct a new RescaleStep
 */
RescaleStep::RescaleStep() {
}

ProcessingStep::state	RescaleStep::do_work() {
	return ProcessingStep::complete;
}

std::string	RescaleStep::what() const {
	return std::string("rescaling the pixels");
}

} // namespace process
} // namespace astro
