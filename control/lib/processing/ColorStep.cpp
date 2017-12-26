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
ColorStep::ColorStep() {
}

ProcessingStep::state	ColorStep::do_work() {
	return ProcessingStep::complete;
}

std::string	ColorStep::what() const {
	return std::string("Color correction");
}

} // namespace process
} // namespace astro
