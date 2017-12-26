/*
 * ParseHDRStep.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroProcess.h>
#include "ProcessorParser.h"

namespace astro {
namespace process {

void	ProcessorParser::startHDR(const attr_t& attrs) {
	// create the stacking step
	HDRStep	*s = new HDRStep();
	ProcessingStepPtr	step(s);

	// remember everyhwere
	_stepstack.push(step);

	// parse attributes
	attr_t::const_iterator  i;
        if (attrs.end() != (i = attrs.find("deemphasize"))) {
                s->degree(std::stod(i->second));
                debug(LOG_DEBUG, DEBUG_LOG, 0, "set deemphasize to %f",
                        s->degree());
        }
        if (attrs.end() != (i = attrs.find("radius"))) {
                s->radius(std::stod(i->second));
                debug(LOG_DEBUG, DEBUG_LOG, 0, "set radius to %f",
                        s->radius());
        }
        if (attrs.end() != (i = attrs.find("mask"))) {
	std::string     maskname = i->second;
		ProcessingStepPtr       maskstep = _network->byname(maskname);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"mask attribute found: %s, step %d",
			maskname.c_str(), maskstep->id());
		s->maskid(maskstep->id());
		step->add_precursor(maskstep);
	}

	startCommon(attrs);
}

} // namespace process
} // namespace astro
