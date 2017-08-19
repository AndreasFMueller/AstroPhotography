/*
 * ParserSteps.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include "ProcessorParser.h"

namespace astro {
namespace process {

/**
 * \brief Create a new File image node
 */
void	ProcessorParser::startFileimage(const attr_t& attrs) {
	// get the file name
	attr_t::const_iterator	i = attrs.find(std::string("file"));
	if (i == attrs.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no file name");
		throw std::runtime_error("no file name");
	}
	std::string	filename = i->second;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filename: %s", filename.c_str());
	RawImageFileStep	*filestep = new RawImageFileStep(filename);
	ProcessingStepPtr	step(filestep);
	_network->add(step);

	// check whether there is a name attribute
	i = attrs.find(std::string("name"));
	if (i != attrs.end()) {
		step->name(i->second);
	}

	// check whether we have something on the stack to which we
	// can add this as a precursor
	if (_stack.size() > 0) {
		// add precursor
		_stack.top()->add_precursor(step);
	}

	// push the process on the stack
	_stack.push(step);
	_network->add(step);
}

void	ProcessorParser::endCommon() {
	ProcessingStepPtr	step = _stack.top();
	_stack.pop();
	if (_stack.top()) {
		_stack.top()->add_precursor(step);
	}
}

void	ProcessorParser::endFileimage() {
	endCommon();
}

void	ProcessorParser::startDarkimage(const attr_t& /* attrs */) {
	DarkProcessorStep	*dark = new DarkProcessorStep();
	ProcessingStepPtr	step(dark);
	_stack.push(step);
}

void	ProcessorParser::endDarkimage() {
	endCommon();
}

void	ProcessorParser::startFlatimage(const attr_t& attrs) {
	FlatProcessorStep	*flat = new FlatProcessorStep();
	ProcessingStepPtr	step(flat);
	attr_t::const_iterator	i = attrs.find(std::string("dark"));
	if (attrs.end() != i) {
		std::string	darkname = i->second;
		ProcessingStepPtr	darkstep = _network->byname(darkname);
		step->add_precursor(darkstep);
	}
	
	_network->add(step);
	_stack.push(step);
}

void	ProcessorParser::endFlatimage() {
	endCommon();
}

void	ProcessorParser::startCalibrate(const attr_t& attrs) {
	ImageCalibrationStep	*cal = new ImageCalibrationStep();
	ProcessingStepPtr	step(cal);
	_network->add(step);
	// get the dark image
	attr_t::const_iterator	i = attrs.find(std::string("dark"));
	if (attrs.end() != i) {
		std::string	name = i->second;
		ProcessingStepPtr	dark = _network->bynameid(name);
		cal->add_precursor(dark);
	}
	// get the flat image
	i = attrs.find(std::string("flat"));
	if (attrs.end() != i) {
		std::string	name = i->second;
		ProcessingStepPtr	flat = _network->bynameid(name);
		cal->add_precursor(flat);
	}
	_stack.push(step);
}

void	ProcessorParser::endCalibrate() {
	endCommon();
}

} // namespace process
} // namespace astro
