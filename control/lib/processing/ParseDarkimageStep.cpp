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
		dark->detect_bad_pixels(true);
	}

	i = attrs.find(std::string("interpolate"));
	if (i != attrs.end()) {
		if ((i->second == "yes") || (i->second == "true"))
			dark->interpolate(true);
	}

	i = attrs.find(std::string("absolute"));
	if (i != attrs.end()) {
		dark->absolute(std::stoi(i->second));
		dark->detect_bad_pixels(true);
	}

	i = attrs.find(std::string("detect_bad_pixels"));
	if (i != attrs.end()) {
		if ((i->second == "yes") || (i->second == "true"))
			dark->detect_bad_pixels(true);
	}

	// remember the step everywhere
	push(step);

	startCommon(attrs);
}

} // namespace process
} // namespace astro
