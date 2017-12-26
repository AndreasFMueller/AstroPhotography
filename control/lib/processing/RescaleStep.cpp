/*
 * RescaleStep.cpp -- implementation of the Rescale step
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <sstream>

namespace astro {
namespace process {

/**
 * \brief Construct a new RescaleStep
 */
RescaleStep::RescaleStep() {
}

ProcessingStep::state	RescaleStep::do_work() {
	switch (status()) {
	case ProcessingStep::needswork:
	case ProcessingStep::complete:
		return ProcessingStep::complete;
	default:
		return ProcessingStep::idle;
	}
}

std::string	RescaleStep::what() const {
	std::ostringstream	out;
	out << "rescale: ";
	if (minimum() >= 0) {
		out << "minimum = " << minimum() << " ";
	}
	if (maximum() >= 0) {
		out << "maximum = " << maximum() << " ";
	}
	if (scale() >= 0) {
		out << "scale = " << scale() << " ";
	}
	return out.str();
}

ImagePtr	RescaleStep::image() {
	// process the precursor image using the operator() from the
	// Rescale superclass
	return (*this)(precursorimage());
}

} // namespace process
} // namespace astro
