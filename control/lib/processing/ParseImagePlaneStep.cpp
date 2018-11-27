/*
 * ParseImagePlaneStep.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroProcess.h>
#include "ProcessorParser.h"

namespace astro {
namespace process {

static int	name2plane(const std::string name) {
	try {
		return std::stoi(name);
	} catch (const std::exception& x) {
	}
	if (name == std::string("R")) {
		return 0;
	}
	if (name == std::string("G")) {
		return 1;
	}
	if (name == std::string("B")) {
		return 2;
	}
	if (name == std::string("L")) {
		return 3;
	}
	std::string	msg = stringprintf("cannot convert plane name '%s'",
		name.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Create a new File image node
 *
 * \param attrs		XML attributes specified in the start element
 */
void	ProcessorParser::startImagePlane(const attr_t& attrs) {
	// get the file name
	attr_t::const_iterator	i = attrs.find(std::string("plane"));
	if (i == attrs.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no plane name/number");
		throw std::runtime_error("no plane name/number");
	}
	std::string	planename = i->second;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "planename: %s", planename.c_str());
	int	plane = name2plane(planename);
	if ((plane > 3) || (plane < 0)) {
		std::string	msg = stringprintf("bad plane number %d",
			plane);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	ImagePlaneStep	*planestep = new ImagePlaneStep(nodePaths(), plane);
	ProcessingStepPtr	step(planestep);

	// push the process on the stack
	push(step);

	startCommon(attrs);
}

} // namespace process
} // namespace astro
