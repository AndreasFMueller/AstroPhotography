/*
 * ColorclampStep.cpp -- implementation of the color clamping step
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>

namespace astro {
namespace process {

/**
 * \brief Construct a new ColorclampStep
 */
ColorclampStep::ColorclampStep() {
}

ProcessingStep::state	ColorclampStep::do_work() {
	return ProcessingStep::complete;
}

std::string	ColorclampStep::what() const {
	return std::string("Color clamping");
}

} // namespace process
} // namespace astro
