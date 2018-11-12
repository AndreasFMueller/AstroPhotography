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

void	ProcessorParser::setNodePaths(NodePaths& nodepaths,
		const attr_t& attrs, NodePaths *parent) {
	// handle the processing step
	std::string	_src;
	{
		auto	i = attrs.find(std::string("src"));
		if (i != attrs.end()) {
			_src = i->second;
		}
	}
	std::string	_dst;
	{
		auto	i = attrs.find(std::string("dst"));
		if (i != attrs.end()) {
			_dst = i->second;
		}
	}
	if (parent) {
		nodepaths._srcpath = StepPathPtr(new StepPath(_src,
					parent->_srcpath));
		nodepaths._dstpath = StepPathPtr(new StepPath(_dst,
					parent->_dstpath));
	} else {
		nodepaths._srcpath = StepPathPtr(new StepPath(_src));
		nodepaths._dstpath = StepPathPtr(new StepPath(_dst));
	}

}

/**
 * \brief common method call when a an element begins
 *
 * This should not be called for the process top level element
 *
 * \param attrs		XML attributes specified in the start element
 */
void	ProcessorParser::startCommon(const attr_t& attrs) {
	ProcessingStepPtr	step = top();

	// handle the processing step
#if 0
	std::string	_src;
	{
		auto	i = attrs.find(std::string("src"));
		if (i != attrs.end()) {
			_src = i->second;
		}
	}
	std::string	_dst;
	{
		auto	i = attrs.find(std::string("dst"));
		if (i != attrs.end()) {
			_dst = i->second;
		}
	}
	if (_parent) {
		step->_srcpath = StepPathPtr(new StepPath(_src,
					_parent->_srcpath));
		step->_dstpath = StepPathPtr(new StepPath(_dst,
					_parent->_dstpath));
	} else {
		step->_srcpath = StepPathPtr(new StepPath(_src));
		step->_dstpath = StepPathPtr(new StepPath(_dst));
	}
#else
	if (_parent) {
		setNodePaths(*step, attrs, &*_parent);
	} else {
		setNodePaths(*step, attrs, &*_network);
	}
#endif

	// check the name attribute
	auto i = attrs.find(std::string("name"));
	if (i != attrs.end()) {
		step->name(i->second);
	} else {
		step->name(generate_name());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "name of %d node: %s",
		step->id(), step->name().c_str());

	// if there is a current top element, then add the present element
	// as a precursor to the top of stack
	if (_parent) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"add precursor %s(%d) to %s(%d)",
			step->name().c_str(), step->id(),
			_parent->name().c_str(), _parent->id());
		_parent->add_precursor(step);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s has now %d precursors",
			_parent->name().c_str(), _parent->precursors().size());
	}

	// remember the step in the network
	ProcessingStep::remember(step);
	_network->add(step);

	// add precursor image if present
	if (attrs.end() != (i = attrs.find("image"))) {
		std::string     imagename = i->second;
		ProcessingStepPtr       imagestep = _network->byname(imagename);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"image attribute found: %s, step %d",
			imagename.c_str(), imagestep->id());
		step->add_precursor(imagestep);
        }
}

/**
 * \brief common method to call when an element ends
 */
void	ProcessorParser::endCommon() {
	ProcessingStepPtr	step = top();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "endCommon() called, %d on stack, %s",
		_stepstack.size(), step->name().c_str());
	pop();
}

/**
 * \brief Start a new process description
 *
 * \param attrs		XML attributes of the flat image element
 */
void	ProcessorParser::startProcess(const attr_t& attrs) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start process description");
	_network = ProcessorNetworkPtr(new ProcessorNetwork());
	setNodePaths(*_network, attrs);
}

/**
 * \brief End of process
 */
void	ProcessorParser::endProcess() {
}

} // namespace process
} // namespace astro
