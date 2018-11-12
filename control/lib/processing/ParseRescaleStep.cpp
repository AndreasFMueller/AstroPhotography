/*
 * ParseRescaleStep.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroProcess.h>
#include "ProcessorParser.h"

namespace astro {
namespace process {

void	ProcessorParser::startRescale(const attr_t& attrs) {
	// create the stacking step
	RescaleStep	*s = new RescaleStep();
	ProcessingStepPtr	step(s);

	// remember everyhwere
	push(step);

	// parse the Rescale attributes
	attr_t::const_iterator	i;
	if (attrs.end() != (i = attrs.find("minimum"))) {
		s->minimum(std::stod(i->second));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set minimum to %f",
			s->minimum());
	}
	if (attrs.end() != (i = attrs.find("maximum"))) {
		s->maximum(std::stod(i->second));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set maximum to %f",
			s->maximum());
	}
	if (attrs.end() != (i = attrs.find("scale"))) {
		s->scale(std::stod(i->second));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set scale to %f",
			s->scale());
	}

	startCommon(attrs);
}

} // namespace process
} // namespace astro
