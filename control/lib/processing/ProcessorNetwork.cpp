/*
 * ProcessorNetwork.cpp -- processor network implementation
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <AstroExceptions.h>

namespace astro {
namespace process {

/**
 * \brief Construct a new processor network
 */
ProcessorNetwork::ProcessorNetwork() {
	_maxthreads = 1;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a new processor network");
}

/**
 * \brief Add a processing step to a network
 */
void	ProcessorNetwork::add(ProcessingStepPtr step) {
	if (!step) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "empty step");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding new step of type %s",
		astro::demangle(typeid(*step).name()).c_str());
	// add the step
	int	id = step->id();
	_steps.insert(std::make_pair(id, step));

	// check whether there is some name to add
	std::string	name = step->name();
	if (name.size() == 0) {
		return;
	}
	_id2names.insert(std::make_pair(id, name));
	_name2ids.insert(std::make_pair(name, id));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add name %s", name.c_str());
}

/**
 * \brief Retrieve a step from the network by id
 */
ProcessingStepPtr	ProcessorNetwork::byid(int id) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for step with id %d", id);
	stepmap_t::const_iterator	i = _steps.find(id);
	if (i != _steps.end()) {
		return i->second;
	}
	std::string	msg = stringprintf("step %d not found", id);
	throw NotFound(msg);
}

/**
 * \brief Retrieve a step from the network by name
 */
ProcessingStepPtr	ProcessorNetwork::byname(const std::string& name) const {
	int	n = _name2ids.count(name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for step '%s': %d entries",
		name.c_str(), n);
	if (n == 0) {
		std::string	msg = stringprintf("no step named '%s'",
			name.c_str());
		throw NotFound(msg);
	}
	if (n > 1) {
		std::string	msg = stringprintf("%d steps named '%s'",
			n, name.c_str());
		throw NotFound(msg);
	}
	name2idmap_t::const_iterator	i = _name2ids.find(name);
	return _steps.find(i->second)->second;
}

ProcessingStepPtr	ProcessorNetwork::bynameid(const std::string& name) const {
	if (name.size() == 0) {
		std::string	msg = stringprintf("no step named '%s'",
			name.c_str());
		throw NotFound(msg);
	}
	if (name[0] == '#') {
		return byid(std::stoi(name.substr(1)));
	}
	return byname(name);
}

/**
 * \brief Build a set of Terminal processing nodes
 *
 * Terminal processing nodes are ones that have no successors
 */
ProcessingStep::steps	ProcessorNetwork::terminals() const {
	ProcessingStep::steps	result;
	std::for_each(_steps.begin(), _steps.end(),
		[&result](const std::pair<int, ProcessingStepPtr>& p) mutable {
			if (p.second->successorCount() == 0) {
				result.push_back(p.first);
			}
		}
	);
	return result;
}

/**
 * \brief Build a set of initial processing nodes
 *
 * initial processing nodes are the ones that have no predecessores
 */
ProcessingStep::steps	ProcessorNetwork::initials() const {
	ProcessingStep::steps	result;
	std::for_each(_steps.begin(), _steps.end(),
		[&result](const std::pair<int, ProcessingStepPtr>& p) mutable {
			if (p.second->precursorCount() == 0) {
				result.push_back(p.first);
			}
		}
	);
	return result;
}

void	ProcessorNetwork::checkstate() {
	std::for_each(_steps.begin(), _steps.end(),
		[](const std::pair<int, ProcessingStepPtr>& p) mutable {
			ProcessingStepPtr       step = p.second;
			step->checkyourstate();
		}
	);
}

bool	ProcessorNetwork::hasneedswork() {
	stepmap_t::const_iterator	i;
	i = std::find_if(_steps.begin(), _steps.end(),
		[](const std::pair<int, ProcessingStepPtr>& p) ->bool {
			return ProcessingStep::needswork == p.second->status();
		}
	);
	return (i != _steps.end());
}

/**
 * \brief finds the topmost node that needs work
 */
int	ProcessorNetwork::process(int id) {
	// check the current node
	ProcessingStepPtr	current = ProcessingStep::byid(id);
	ProcessingStep::state	s = current->status();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "process(%d, %s, %d) %s", id,
		current->name().c_str(), s,
		demangle(typeid(*current).name()).c_str());
	switch (s) {
		case ProcessingStep::needswork:
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%d needs work", id);
			return id;
		case ProcessingStep::idle:
			// check all precursors
			return process(current->precursors());
		// never check below working, complete or failed nodes
		case ProcessingStep::working:
		case ProcessingStep::complete:
		case ProcessingStep::failed:
			return -1;
	}
	throw std::runtime_error("should not happen");
}

/**
 * \brief Check a list of steps for a possible node that needs work
 */
int	ProcessorNetwork::process(const ProcessingStep::steps& steps) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking %lu steps for work",
		steps.size());
	ProcessingStep::steps::const_iterator	i;
	for (i = steps.begin(); i != steps.end(); i++) {
		int	id = process(*i);
		if (id >= 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"found %d in need of work", id);
			return id;
		}
	}
	return -1;
}

/**
 * \brief Process the complete network
 */
void	ProcessorNetwork::process() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start processing");
	checkstate();
	ProcessingStep::steps	t = terminals();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d terminals", t.size());
	int	id;
	while (0 <= (id = process(t))) {
		ProcessingStepPtr	step = ProcessingStep::byid(id);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "working on %d '%s'", id,
			step->name().c_str());
		step->work();
	} 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end processing");
}

} // namespace process
} // namespace astro
