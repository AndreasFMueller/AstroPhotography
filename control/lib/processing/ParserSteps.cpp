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

static int	namenumber = 0;

static std::string	generate_name() {
	return stringprintf("step%d", namenumber++);
}

/**
 * \brief common method call when a an element begins
 *
 * This should not be called for the process top level element
 *
 * \param attrs		XML attributes specified in the start element
 */
void	ProcessorParser::startCommon(const attr_t& attrs) {
	ProcessingStepPtr	step = _stepstack.top();

	// check the base attribute
	attr_t::const_iterator	i = attrs.find(std::string("base"));
	if (i != attrs.end()) {
		std::string	newbase = _basestack.top() + "/" + i->second;
		_basestack.push(newbase);
	} else {
		_basestack.push(_basestack.top());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "current top base: '%s'",
		_basestack.top().c_str());

	// check the name attribute
	i = attrs.find(std::string("name"));
	if (i != attrs.end()) {
		_namestack.push(i->second);
	} else {
		_namestack.push(generate_name());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "name of this node: %s",
		step->name().c_str());

	// remember the step in the network
	ProcessingStep::remember(step);
	_network->add(step);
}

/**
 * \brief common method to call when an element ends
 */
void	ProcessorParser::endCommon() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "endCommon() called");

	// get the step that was pushed on the stack with the start element
	ProcessingStepPtr	step = _stepstack.top();
	step->name(_namestack.top());
	_stepstack.pop();
	_namestack.pop();

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
		if (NULL == getcwd(buffer, sizeof(buffer))) {
			throw std::runtime_error("cannot get cwd");
		}
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
