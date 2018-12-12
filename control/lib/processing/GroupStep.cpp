/*
 * GroupStep.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>

namespace astro {
namespace process {

/**
 * \brief The work is always complete
 */
ProcessingStep::state	GroupStep::do_work() {
	return ProcessingStep::complete;
}

/**
 * \brief Inform about grouping
 */
std::string	GroupStep::what() const {
	return std::string("grouping step");
}

} // namespace process
} // namespace astro
