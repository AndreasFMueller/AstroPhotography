/*
 * ParseColorStep.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroProcess.h>
#include "ProcessorParser.h"

namespace astro {
namespace process {

void	ProcessorParser::startColor(const attr_t& attrs) {
	// create the stacking step
	ColorStep	*s = new ColorStep();
	ProcessingStepPtr	step(s);

	// remember everyhwere
	push(step);

	// parse attributes
	attr_t::const_iterator	i;
	if (attrs.end() != (i = attrs.find("gain"))) {
		s->gain(std::stod(i->second));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set gain to %f",
			s->gain());
	}
	if (attrs.end() != (i = attrs.find("brightness"))) {
		s->base(std::stod(i->second));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set brightness to %f",
			s->base());
	}
	if (attrs.end() != (i = attrs.find("limit"))) {
		s->limit(std::stod(i->second));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set limit to %f",
			s->limit());
	}
	if (attrs.end() != (i = attrs.find("scales"))) {
		s->scales(i->second);
		RGB<double>	sc = s->scales();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set scales to %.2f,%.2f,%.2f", 
			sc.R, sc.G, sc.B);
	}
	if (attrs.end() != (i = attrs.find("offsets"))) {
		s->offsets(i->second);
		RGB<double>	sc = s->offsets();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set offsets to %.2f,%.2f,%.2f", 
			sc.R, sc.G, sc.B);
	}

	startCommon(attrs);
}

} // namespace process
} // namespace astro
