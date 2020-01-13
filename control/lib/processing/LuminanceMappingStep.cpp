/*
 * LuminanceMappingStep.cpp -- implementation of the color step
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>

namespace astro {
namespace process {

/**
 * \brief Construct a new LuminanceMappingStep
 */
LuminanceMappingStep::LuminanceMappingStep(NodePaths& parent)
	: ImageStep(parent) {
}

adapter::LuminanceFunctionPtr	LuminanceMappingStep::luminancefunctionptr() const {
	return _luminancefunctionptr;
}

void	LuminanceMappingStep::luminancefunctionptr(adapter::LuminanceFunctionPtr l) {
	_luminancefunctionptr = l;
}

ProcessingStep::state	LuminanceMappingStep::do_work() {
	switch (status()) {
	case ProcessingStep::needswork:
	case ProcessingStep::complete:
		return ProcessingStep::complete;
	default:
		return ProcessingStep::idle;
	}
}

std::string	LuminanceMappingStep::what() const {
	if (_luminancefunctionptr) {
		return stringprintf("LuminanceMapping with %s",
			_luminancefunctionptr->info().c_str());
	}
	return std::string("LuminanceMapping correction");
}

ImagePtr	LuminanceMappingStep::image() {
	ImagePtr	precursor = precursorimage();
	ImagePtr	result = adapter::luminancemapping(precursor,
					_luminancefunctionptr);
	return result;
}

} // namespace process
} // namespace astro
