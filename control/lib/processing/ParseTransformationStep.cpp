/*
 * ParseStackStep.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <ProcessorParser.h>

namespace astro {
namespace process {

/**
 * \brief start the stacking step
 */
void	ProcessorParser::startTransform(const attr_t& attrs) {
	// create the stacking step
	ImageTransformationStep	*its = new ImageTransformationStep(nodePaths());
	ProcessingStepPtr	step(its);

	// remember everyhwere
	push(step);

	// get the attributes for the stacking step
	attr_t::const_iterator	i;
	if (attrs.end() != (i = attrs.find("vertical_flip"))) {
		std::string	value = i->second;
		if ((value == "no") || (value == "false")) {
			its->vertical_flip(false);
		} else {
			its->vertical_flip(true);
		}
	}
	if (attrs.end() != (i = attrs.find("horizontal_flip"))) {
		std::string	value = i->second;
		if ((value == "no") || (value == "false")) {
			its->horizontal_flip(false);
		} else {
			its->horizontal_flip(true);
		}
	}
	if (attrs.end() != (i = attrs.find("upscale"))) {
		std::string	value = i->second;
		its->scale(std::stoi(value));
	}
	if (attrs.end() != (i = attrs.find("downscale"))) {
		std::string	value = i->second;
		its->scale(-std::stoi(value));
	}
	if (attrs.end() != (i = attrs.find("xshift"))) {
		std::string	value = i->second;
		its->xshift(std::stof(value));
	}
	if (attrs.end() != (i = attrs.find("yshift"))) {
		std::string	value = i->second;
		its->yshift(std::stof(value));
	}

	startCommon(attrs);
}

} // namespace process
} // namespace astro
