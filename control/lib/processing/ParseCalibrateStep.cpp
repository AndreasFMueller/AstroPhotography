/*
 * ParseCalibrateStep.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <ProcessorParser.h>

namespace astro {
namespace process {

/**
 * \brief start an image calibration process
 *
 * \param attrs		XML attributes of the flat image element
 */
void	ProcessorParser::startCalibrate(const attr_t& attrs) {
	// create a new image calibration step
	ImageCalibrationStep	*cal = new ImageCalibrationStep();
	ProcessingStepPtr	step(cal);

	// remember the step everywhere
	_stepstack.push(step);

	// get the dark image
	attr_t::const_iterator	i = attrs.find(std::string("dark"));
	if (attrs.end() != i) {
		std::string	darkname = i->second;
		ProcessingStepPtr	dark = _network->bynameid(darkname);
		cal->add_precursor(dark);
		cal->dark(dark);
	}

	// get the flat image
	i = attrs.find(std::string("flat"));
	if (attrs.end() != i) {
		std::string	flatname = i->second;
		ProcessingStepPtr	flat = _network->bynameid(flatname);
		cal->add_precursor(flat);
		cal->flat(flat);
	}

	// check other attributes
	i = attrs.find(std::string("demosaic"));
	if (i != attrs.end()) {
		if ((i->second == std::string("yes"))
			|| (i->second == std::string("true"))) {
			cal->demosaic(true);
		}
	}

	i = attrs.find(std::string("interpolate"));
	if (i != attrs.end()) {
		if ((i->second == std::string("yes"))
			|| (i->second == std::string("true"))) {
			cal->interpolate(true);
		}
	}

	i = attrs.find(std::string("flip"));
	if (i != attrs.end()) {
		if ((i->second == std::string("yes"))
			|| (i->second == std::string("true"))) {
			cal->flip(true);
		}
	}

	startCommon(attrs);
}

} // namespace process
} // namespace astro
