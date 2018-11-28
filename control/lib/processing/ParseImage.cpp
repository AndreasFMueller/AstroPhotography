/*
 * ParseImage.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <ProcessorParser.h>

namespace astro {
namespace process {

void	ProcessorParser::startImage(const attr_t& attrs) {
	// parse attributes
	attr_t::const_iterator  i;
	if (attrs.end() == (i = attrs.find("ref"))) {
		throw std::runtime_error("ref attribute missing");
	}
	std::string	ref = i->second;
	ProcessingStepPtr	step = _network->byname(ref);
	if (!step) {
		std::string	msg = stringprintf("step %s not found",
			ref.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	ProcessingStepPtr	stacktop = _stepstack.top();

	debug(LOG_DEBUG, DEBUG_LOG, 0, "add precursor %s to %s",
		step->verboseinfo().c_str(),
		stacktop->verboseinfo().c_str());

	stacktop->add_precursor(step->id());
	step->add_successor(stacktop->id());

	push(step);
}

} // namespace process
} // namespace astro
