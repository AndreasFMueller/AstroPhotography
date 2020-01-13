/*
 * ParseLuminanceMappingStep.cpp
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "includes.h"
#include <AstroProcess.h>
#include <ProcessorParser.h>
#include <AstroTonemapping.h>

namespace astro {
namespace process {

void	ProcessorParser::startLuminanceMapping(const attr_t& attrs) {
	// get the name of the luminance function that we are looking for
	auto	i = attrs.find("function");
	if (i == attrs.end()) {
		std::string	msg = stringprintf("'function' attribute "
			"missing");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	std::string	name = i->second;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "luminance mapping for function '%s'",
		name.c_str());

	// get the luminance mapping function by name
	adapter::LuminanceFunctionPtr	luminancefunctionptr
		= adapter::LuminanceFunctionFactory::get(name, attrs);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got luminance function: %s",
		luminancefunctionptr->info().c_str());

	// construct the step
	LuminanceMappingStep	*luminancemappingstep
					= new LuminanceMappingStep(nodePaths());
	luminancemappingstep->luminancefunctionptr(luminancefunctionptr);
	ProcessingStepPtr       step(luminancemappingstep);

	// push the process step on the stack
	push(step);

	startCommon(attrs);
}

} // namespace process
} // namespace astro
