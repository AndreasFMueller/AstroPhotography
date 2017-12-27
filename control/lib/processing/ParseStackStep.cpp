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
void	ProcessorParser::startStack(const attr_t& attrs) {
	// create the stacking step
	StackingStep	*ss = new StackingStep();
	ProcessingStepPtr	sstep(ss);

	// remember everyhwere
	_stepstack.push(sstep);

	// we need the baseimage attribute (don't confuse with the base
	// attribute, which relates to the base directory)
	attr_t::const_iterator	i = attrs.find(std::string("baseimage"));
	if (i == attrs.end()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "baseimage attribute missing");
		throw std::runtime_error("missing base image");
	}
	ProcessingStepPtr	bi = _network->bynameid(i->second);
	if (!bi) {
		std::string	msg = stringprintf("referenced base image "
			"'%s' not found", i->second.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	ss->baseimage(bi);

	// get the attributes for the stacking step
	if (attrs.end() != (i = attrs.find("searchradius"))) {
		int	sradius= std::stoi(i->second);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set search radius to %d",
			sradius);
		ss->searchradius(sradius);
	}
	if (attrs.end() != (i = attrs.find("patchsize"))) {
		int	patchsize= std::stoi(i->second);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set patch size to %d",
			patchsize);
		ss->patchsize(patchsize);
	}
	if (attrs.end() != (i = attrs.find("residual"))) {
		double	residual= std::stoi(i->second);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set residual to %f",
			residual);
		ss->residual(residual);
	}
	if (attrs.end() != (i = attrs.find("numberofstars"))) {
		int	numberofstars= std::stoi(i->second);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set number of stars to %d",
			numberofstars);
		ss->numberofstars(numberofstars);
	}
	if (attrs.end() != (i = attrs.find("transform"))) {
		std::string	value = i->second;
		if ((value == "no") || (value == "false")) {
			ss->notransform(false);
		} else {
			ss->notransform(true);
		}
	}
	if (attrs.end() != (i = attrs.find("usetriangles"))) {
		std::string	value = i->second;
		if ((value == "no") || (value == "false")) {
			ss->usetriangles(false);
		} else {
			ss->usetriangles(true);
		}
	}
	if (attrs.end() != (i = attrs.find("rigid"))) {
		std::string	value = i->second;
		if ((value == "no") || (value == "false")) {
			ss->rigid(false);
		} else {
			ss->rigid(true);
		}
	}

	startCommon(attrs);

	if (ss->baseimage()) {
		sstep->add_precursor(ss->baseimage());
	}
}

} // namespace process
} // namespace astro
