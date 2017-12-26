/*
 * HDRStep.cpp -- implementation of the HDR step
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>

namespace astro {
namespace process {

/**
 * \brief Construct a new HDRStep
 */
HDRStep::HDRStep() {
}

ProcessingStep::state	HDRStep::do_work() {
	return ProcessingStep::complete;
}

std::string	HDRStep::what() const {
	return std::string("perform HDR transform");
}

} // namespace process
} // namespace astro
