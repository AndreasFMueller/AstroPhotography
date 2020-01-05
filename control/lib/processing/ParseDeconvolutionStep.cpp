/*
 * ParseDeconvolutionStep.cpp
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "includes.h"
#include <AstroProcess.h>
#include <ProcessorParser.h>

namespace astro {
namespace process {

void	ProcessorParser::startDeconvolution(const attr_t& attrs) {
	// get the file name
	DeconvolutionStep *deconvolutionstep
		= new DeconvolutionStep(nodePaths());
	ProcessingStepPtr       step(deconvolutionstep);
	push(step);

	// read the parameters from the attributes
	attr_t::const_iterator	i;
	if (attrs.end() != (i = attrs.find("psf"))) {
		std::string     psfname = i->second;
		ProcessingStepPtr       psfstep = _network->byname(psfname);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"psf attribute found: %s, step %d",
			psfname.c_str(), psfstep->id());
		deconvolutionstep->psf(psfstep);
	}
	if (attrs.end() != (i = attrs.find("method"))) {
		deconvolutionstep->method(i->second);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set method to %s",
			deconvolutionstep->method().c_str());
	}
	if (attrs.end() != (i = attrs.find("iterations"))) {
		deconvolutionstep->iterations(std::stoi(i->second));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set iterations to %d",
			deconvolutionstep->iterations());
	}
	if (attrs.end() != (i = attrs.find("epsilon"))) {
		deconvolutionstep->epsilon(std::stod(i->second));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set epsilon to %f",
			deconvolutionstep->epsilon());
	}
	if (attrs.end() != (i = attrs.find("K"))) {
		deconvolutionstep->K(std::stod(i->second));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set K to %f",
			deconvolutionstep->K());
	}
	if (attrs.end() != (i = attrs.find("stddev"))) {
		deconvolutionstep->stddev(std::stod(i->second));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set stddev to %f",
			deconvolutionstep->stddev());
	}
	if (attrs.end() != (i = attrs.find("epsilon"))) {
		deconvolutionstep->epsilon(std::stod(i->second));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set epsilon to %f",
			deconvolutionstep->epsilon());
	}

        startCommon(attrs);
        if (deconvolutionstep->psf()) {
                step->add_precursor(deconvolutionstep->psf());
        }
}

} // namespace process
} // namespace astro
