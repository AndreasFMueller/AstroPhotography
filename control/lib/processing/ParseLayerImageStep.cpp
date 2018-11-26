/*
 * ParseLayerImageStep.cpp
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
void	ProcessorParser::startLayerImage(const attr_t& attrs) {
	// get the file name
	LayerImageStep	*layerstep = new LayerImageStep(nodePaths());
	ProcessingStepPtr	step(layerstep);

	// push the process on the stack
	push(step);

	startCommon(attrs);
}

} // namespace process
} // namespace astro
