/*
 * LuminanceStretchingStep.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>

namespace astro {
namespace process {

LuminanceStretchingStep::LuminanceStretchingStep() {
}

ProcessingStep::state	LuminanceStretchingStep::do_work() {
	ImagePtr	precursor = precursorimage();
	_image = adapter::luminancestretching(precursor, *_factor);
	return ProcessingStep::complete;
}

std::string	LuminanceStretchingStep::what() const {
	return std::string("stretching luminance");
}

} // namespace process
} // namespace astro
