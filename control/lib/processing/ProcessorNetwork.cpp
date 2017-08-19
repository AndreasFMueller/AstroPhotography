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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for step '%s'", name.c_str());
	int	n = _name2ids.count(name);
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
std::set<ProcessingStepPtr>	ProcessorNetwork::terminals() const {
	std::set<ProcessingStepPtr>	result;
	std::for_each(_steps.begin(), _steps.end(),
		[&result](const std::pair<int, ProcessingStepPtr>& p) mutable {
			if (p.second->successorCount() == 0) {
				result.insert(p.second);
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
std::set<ProcessingStepPtr>	ProcessorNetwork::initials() const {
	std::set<ProcessingStepPtr>	result;
	std::for_each(_steps.begin(), _steps.end(),
		[&result](const std::pair<int, ProcessingStepPtr>& p) mutable {
			if (p.second->precursorCount() == 0) {
				result.insert(p.second);
			}
		}
	);
	return result;
}

} // namespace process
} // namespace astro
