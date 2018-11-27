/*
 * ParseRGBStep.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroProcess.h>
#include "ProcessorParser.h"

namespace astro {
namespace process {

/**
 * \brief Create a new File image node
 *
 * \param attrs		XML attributes specified in the start element
 */
void	ProcessorParser::startRGB(const attr_t& attrs) {
	// get the file name
	RGBStep	*rgbstep = new RGBStep(nodePaths());
	ProcessingStepPtr	step(rgbstep);

	// push the process on the stack
	push(step);

	startCommon(attrs);
}

} // namespace process
} // namespace astro
