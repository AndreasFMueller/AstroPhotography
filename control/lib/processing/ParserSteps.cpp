/*
 * ParserSteps.cpp -- implementation of the parser steps
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroProcess.h>
#include "ProcessorParser.h"

namespace astro {
namespace process {

/**
 * \brief common method call when a an element begins
 *
 * This should not be called for the process top level element
 *
 * \param attrs		XML attributes specified in the start element
 */
void	ProcessorParser::startCommon(const attr_t& attrs) {
	attr_t::const_iterator	i = attrs.find(std::string("base"));
	if (i != attrs.end()) {
		std::string	newbase = _basestack.top() + "/" + i->second;
		_basestack.push(newbase);
	} else {
		_basestack.push(_basestack.top());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "current top base: '%s'",
		_basestack.top().c_str());
}

/**
 * \brief common method to call when an element ends
 */
void	ProcessorParser::endCommon() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "endCommon() called");

	// get the step that was pushed on the stack with the start element
	ProcessingStepPtr	step = _stepstack.top();
	_stepstack.pop();

	// if there is a current top element, then add the present element
	// as a precursor to the top of stack
	if (_stepstack.size() > 0) {
		if (_stepstack.top()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "add precursor %d to %d",
				step->id(), _stepstack.top()->id());
			_stepstack.top()->add_precursor(step);
		}
	}

	// remove the top element from the base path stack
	if (_basestack.size() > 0) {
		_basestack.pop();
	}
}

/**
 * \brief Create a new File image node
 *
 * \param attrs		XML attributes specified in the start element
 */
void	ProcessorParser::startFileimage(const attr_t& attrs) {
	startCommon(attrs);
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
	ProcessingStep::remember(step);
	_network->add(step);

	// check whether there is a name attribute
	i = attrs.find(std::string("name"));
	if (i != attrs.end()) {
		step->name(i->second);
	}

	// push the process on the stack
	_stepstack.push(step);
}

/**
 * \brief method called when a file image element ends
 */
void	ProcessorParser::endFileimage() {
	endCommon();
}

/**
 * \brief method called to start a dark image processor
 *
 * \param attrs		XML attributes specified in the start element
 */
void	ProcessorParser::startDarkimage(const attr_t& attrs) {
	startCommon(attrs);

	// create a new dark process
	DarkProcessorStep	*dark = new DarkProcessorStep();
	ProcessingStepPtr	step(dark);

	// remember the step everywhere
	_stepstack.push(step);
	ProcessingStep::remember(step);
	_network->add(step);

	// check whether there is a name attribute
	attr_t::const_iterator	i = attrs.find(std::string("name"));
	if (i != attrs.end()) {
		step->name(i->second);
	}
}

/**
 * \brief method called to end a dar image processor
 */
void	ProcessorParser::endDarkimage() {
	endCommon();
}

/**
 * \brief Start a flat image processor
 *
 * \param attrs		XML attributes of the flat image element
 */
void	ProcessorParser::startFlatimage(const attr_t& attrs) {
	startCommon(attrs);

	// create a new flat process
	FlatProcessorStep	*flat = new FlatProcessorStep();
	ProcessingStepPtr	step(flat);

	// remember the step everywhere
	_stepstack.push(step);
	ProcessingStep::remember(step);
	_network->add(step);

	// add a dark image if the dark attribute is present
	attr_t::const_iterator	i = attrs.find(std::string("dark"));
	if (attrs.end() != i) {
		std::string	darkname = i->second;
		ProcessingStepPtr	darkstep = _network->byname(darkname);
		step->add_precursor(darkstep);
	}
	
	// check whether there is a name attribute
	i = attrs.find(std::string("name"));
	if (i != attrs.end()) {
		step->name(i->second);
	}
}

/**
 * \brief end a flat image processor
 */
void	ProcessorParser::endFlatimage() {
	endCommon();
}

/**
 * \brief start an image calibration process
 *
 * \param attrs		XML attributes of the flat image element
 */
void	ProcessorParser::startCalibrate(const attr_t& attrs) {
	startCommon(attrs);

	// create a new image calibration step
	ImageCalibrationStep	*cal = new ImageCalibrationStep();
	ProcessingStepPtr	step(cal);

	// remember the step everywhere
	_stepstack.push(step);
	ProcessingStep::remember(step);
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

	// check whether there is a name attribute
	i = attrs.find(std::string("name"));
	if (i != attrs.end()) {
		step->name(i->second);
	}
}

/**
 * \brief end an image calibration process
 */
void	ProcessorParser::endCalibrate() {
	endCommon();
}

/**
 * \brief Start a new process description
 *
 * \param attrs		XML attributes of the flat image element
 */
void	ProcessorParser::startProcess(const attr_t& attrs) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start process description");
	if (_basestack.size() > 0) {
		throw std::runtime_error("process must be top level element");
	}
	_network = ProcessorNetworkPtr(new ProcessorNetwork());

	// create the top entry of the base stack
	attr_t::const_iterator	i = attrs.find(std::string("base"));
	if (i == attrs.end()) {
		char	buffer[PATH_MAX];
		getcwd(buffer, sizeof(buffer));
		_basestack.push(std::string(buffer));
	} else {
		_basestack.push(i->second);
	}
	struct stat	sb;
	if (stat(_basestack.top().c_str(), &sb) != 0) {
		std::string	msg = stringprintf("cannot stat '%s': %s",
			_basestack.top().c_str(), strerror(errno));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	if (!S_ISDIR(sb.st_mode)) {
		std::string	msg = stringprintf("'%s' is not a directory",
			_basestack.top().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "base dir: %s",
		_basestack.top().c_str());
}

/**
 * \brief End of process
 */
void	ProcessorParser::endProcess() {
	_basestack.pop();
}

} // namespace process
} // namespace astro
