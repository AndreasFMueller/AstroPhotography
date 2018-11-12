/*
 * ParseColorclampStep.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroProcess.h>
#include "ProcessorParser.h"

namespace astro {
namespace process {

void	ProcessorParser::startColorclamp(const attr_t& attrs) {
	// create the stacking step
	ColorclampStep	*s = new ColorclampStep();
	ProcessingStepPtr	step(s);

	// remember everyhwere
	push(step);

	// parse the Colorclamp attributes
	attr_t::const_iterator  i;
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

	startCommon(attrs);
}

} // namespace process
} // namespace astro
