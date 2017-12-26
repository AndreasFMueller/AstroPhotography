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
 */
DestarStep::DestarStep() {
}

ProcessingStep::state	DestarStep::do_work() {
	return ProcessingStep::complete;
}

std::string	DestarStep::what() const {
	return std::string("Destar an image");
}

} // namespace process
} // namespace astro
