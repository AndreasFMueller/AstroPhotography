/*
 * ParseDarkimageStep.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <ProcessorParser.h>

namespace astro {
namespace process {

/**
 * \brief method called to start a dark image processor
 *
 * \param attrs		XML attributes specified in the start element
 */
void	ProcessorParser::startDarkimage(const attr_t& attrs) {
	// create a new dark process
	DarkImageStep	*dark = new DarkImageStep();
	ProcessingStepPtr	step(dark);

	// remember the step everywhere
	_stepstack.push(step);

	startCommon(attrs);
}

} // namespace process
} // namespace astro
