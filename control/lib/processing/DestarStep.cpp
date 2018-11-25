/*
 * DestarStep.cpp -- implementation of the destarring step
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>

namespace astro {
namespace process {

/**
 * \brief Construct a new DestarStep
 *
 * set the default radius for destarring to 10 pixels
 */
DestarStep::DestarStep(NodePaths& parent) : ImageStep(parent), _radius(10) {
}

/**
 * \brief Work function for destarring
 */
ProcessingStep::state	DestarStep::do_work() {
	try {
		ImagePtr	precursor = precursorimage();
		_image = adapter::destarptr(precursor, radius());
		return ProcessingStep::complete;
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "processing error: %s", x.what());
	}
	return ProcessingStep::failed;
}

/**
 * \brief Inform about what we are doing
 */
std::string	DestarStep::what() const {
	return std::string("Destar an image");
}

} // namespace process
} // namespace astro
