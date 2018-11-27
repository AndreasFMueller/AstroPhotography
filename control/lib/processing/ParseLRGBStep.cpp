/*
 * ParseLRGBStep.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "includes.h"
#include <AstroProcess.h>
#include <ProcessorParser.h>

namespace astro {
namespace process {

void	ProcessorParser::startLRGB(const attr_t& attrs) {
	// get the file name
	LRGBStep *lrgbstep = new LRGBStep(nodePaths());
	ProcessingStepPtr       step(lrgbstep);

	// push the process on the stack
	push(step);

	startCommon(attrs);
}

} // namespace process
} // namespace astro
