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
void	ProcessorParser::startDarkimage(const attr_t& attrs,
		camera::Exposure::purpose_t purpose) {
	// create a new dark process
	DarkImageStep	*dark = new DarkImageStep(nodePaths(), purpose);
	ProcessingStepPtr	step(dark);

	// attribute stdeevs
	auto    i = attrs.find(std::string("stddevs"));
	if (i != attrs.end()) {
		dark->badpixellimit(std::stod(i->second));
	}

	// remember the step everywhere
	push(step);

	startCommon(attrs);
}

} // namespace process
} // namespace astro
