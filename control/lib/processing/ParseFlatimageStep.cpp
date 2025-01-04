/*
 * ParseFlatImageStep.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <ProcessorParser.h>

namespace astro {
namespace process {

/**
 * \brief Start a flat image processor
 *
 * \param attrs		XML attributes of the flat image element
 */
void	ProcessorParser::startFlatimage(const attr_t& attrs) {
	// create a new flat process
	FlatImageStep	*flat = new FlatImageStep(nodePaths());
	ProcessingStepPtr	step(flat);

	// remember the step everywhere
	push(step);

	// add a dark image if the dark attribute is present
	attr_t::const_iterator	i = attrs.find(std::string("dark"));
	if (attrs.end() != i) {
		std::string	darkname = i->second;
		ProcessingStepPtr	darkstep = _network->byname(darkname);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"dark attribute found: %s, step %d",
			darkname.c_str(), darkstep->id());
		flat->dark(darkstep);
	}

	i = attrs.find(std::string("mosaic"));
	if (attrs.end() != i) {
		std::string	mosaicvalue = i->second;
		if (mosaicvalue == std::string("yes")) {
			flat->mosaic(true);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "mosaic = %s",
			(flat->mosaic()) ? "yes" : "no");
	}
	
	startCommon(attrs);
	if (flat->dark()) {
		step->add_precursor(flat->dark());
	}
}

} // namespace process
} // namespace astro
