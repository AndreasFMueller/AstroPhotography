/*
 * ParseGammaStep.cpp
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "includes.h"
#include <AstroProcess.h>
#include <ProcessorParser.h>

namespace astro {
namespace process {

void	ProcessorParser::startGamma(const attr_t& attrs) {
	// get the file name
	GammaStep *gammastep = new GammaStep(nodePaths());
	ProcessingStepPtr       step(gammastep);

	// read the parameters from the attributes
	attr_t::const_iterator	i;
	if (attrs.end() != (i = attrs.find("minimum"))) {
		gammastep->minimum(std::stod(i->second));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set minimum to %f",
			gammastep->minimum());
	}
	if (attrs.end() != (i = attrs.find("maximum"))) {
		gammastep->maximum(std::stod(i->second));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set maximum to %f",
			gammastep->maximum());
	}
	if (attrs.end() != (i = attrs.find("gamma"))) {
		gammastep->gamma(std::stod(i->second));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set gamma to %f",
			gammastep->gamma());
	}

	// push the process on the stack
	push(step);

	startCommon(attrs);
}

} // namespace process
} // namespace astro
