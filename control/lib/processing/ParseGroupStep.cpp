/*
 * ParseGroupStep.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroProcess.h>
#include "ProcessorParser.h"

namespace astro {
namespace process {

void	ProcessorParser::startGroup(const attr_t& attrs) {
	GroupStep	*groupstep = new GroupStep(nodePaths());
	ProcessingStepPtr	step(groupstep);
	push(step);
	startCommon(attrs);
}

} // namespace process
} // namespace astro
