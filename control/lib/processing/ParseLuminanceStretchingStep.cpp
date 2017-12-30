/*
 * ParseLuminanceStretchingStep.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroProcess.h>
#include "ProcessorParser.h"

namespace astro {
namespace process {

void	ProcessorParser::startLuminanceStretching(const attr_t& attrs) {
	// create the stacking step
	LuminanceStretchingStep	*s = new LuminanceStretchingStep();
	ProcessingStepPtr	step(s);

	// remember everyhwere
	_stepstack.push(step);

	double	crossover = 128;
	double	top = 256;
	double	maximum = 65535;

	// parse attributes
	attr_t::const_iterator	i;
	if (attrs.end() != (i = attrs.find("crossover"))) {
		crossover = std::stod(i->second);
	}
	if (attrs.end() != (i = attrs.find("top"))) {
		top = std::stod(i->second);
	}
	if (attrs.end() != (i = attrs.find("maximum"))) {
		maximum = std::stod(i->second);
	}
	adapter::LuminanceFactorPtr	factorptr(
		new adapter::LinearLogLuminanceFactor(crossover, top, maximum));
	s->factor(factorptr);

	startCommon(attrs);
}

} // namespace process
} // namespace astro
