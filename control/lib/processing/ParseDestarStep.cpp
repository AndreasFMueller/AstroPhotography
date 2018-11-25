/*
 * ParseDestarStep.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroProcess.h>
#include "ProcessorParser.h"

namespace astro {
namespace process {

void	ProcessorParser::startDestar(const attr_t& attrs) {
	// create the stacking step
	DestarStep	*s = new DestarStep(nodePaths());
	ProcessingStepPtr	step(s);

	// remember everyhwere
	push(step);

	// parse attributes
	attr_t::const_iterator	i;
	if (attrs.end() != (i = attrs.find("radius"))) {
		s->radius(std::stod(i->second));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set radius to %f",
			s->radius());
	}

	startCommon(attrs);
}

} // namespace process
} // namespace astro
