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
	_stepstack.push(step);

	startCommon(attrs);
}

} // namespace process
} // namespace astro
