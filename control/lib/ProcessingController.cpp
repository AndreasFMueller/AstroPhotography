/*
 * ProcessingController.cpp -- controller to control the processing of a
 *                             network of interdependent processing steps
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>

namespace astro {
namespace process {

//////////////////////////////////////////////////////////////////////
// Construction an destruction
//////////////////////////////////////////////////////////////////////
/**
 * \brief Construct a processing controller
 */
ProcessingController::ProcessingController() {
}

/**
 * \brief Destroy a processing controller
 */
ProcessingController::~ProcessingController() {
}

//////////////////////////////////////////////////////////////////////
// adding and removing steps
//////////////////////////////////////////////////////////////////////
/**
 * \brief add a processing step to the map
 */
void	ProcessingController::addstep(const std::string& name,
		ProcessingStepPtr step) {
	if (steps.find(name) != steps.end()) {
		throw std::runtime_error("duplicate processing step name");
	}
	ProcessingThreadPtr	thread = ProcessingThread::get(step);
	steps.insert(std::make_pair(name, thread));
}

/**
 * \brief Remove a processing step from the map
 */
void	ProcessingController::removestep(const std::string& name) {
	if (steps.find(name) == steps.end()) {
		throw std::runtime_error("no such processing step");
	}
	steps.erase(steps.find(name));
}

/**
 * \brief Find the name of a processing step
 */
std::string	ProcessingController::name(ProcessingStepPtr step) {
	stepmap::iterator	s = std::find_if(steps.begin(), steps.end(),
		[step](stepmap::value_type& v) {
			return (v.second->step() == step);
		}
	);
	if (s == steps.end()) {
		throw std::runtime_error("step not found");
	}
	return s->first;
}

/**
 * \brief Find a step by name
 */
ProcessingStepPtr	ProcessingController::find(const std::string& name) {
	stepmap::iterator	i = steps.find(name);
	if (i == steps.end()) {
		throw std::runtime_error("step named " + name + " not found");
	}
	return i->second->step();
}

//////////////////////////////////////////////////////////////////////
// Successors and predecessors
//////////////////////////////////////////////////////////////////////
void	ProcessingController::add_precursor(const std::string& target_name,
		const std::string& precursor_name) {
	find(target_name)->add_precursor(find(precursor_name));
}

void	ProcessingController::add_successor(const std::string& target_name,
		const std::string& successor_name) {
	find(target_name)->add_successor(find(successor_name));
}

void	ProcessingController::remove_precursor(const std::string& target_name,
		const std::string& precursor_name) {
	find(target_name)->remove_precursor(find(precursor_name));
}

void	ProcessingController::remove_successor(const std::string& target_name,
		const std::string& successor_name) {
	find(target_name)->remove_successor(find(successor_name));
}

//////////////////////////////////////////////////////////////////////
// execution
//////////////////////////////////////////////////////////////////////
bool	ProcessingController::haswork() {
	return std::any_of(steps.begin(), steps.end(),
		[](stepmap::value_type& v) {
			return (v.second->step()->status()
				== ProcessingStep::needswork);
		}
	);
}

void	ProcessingController::execute() {
	while (haswork()) {
		stepmap::iterator	i = std::find_if(steps.begin(),
			steps.end(),
			[](stepmap::value_type& v) {
				return (v.second->step()->status()
					== ProcessingStep::needswork);
			}
		);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "starting %s", i->first.c_str());
		i->second->run();
		i->second->wait();
	}
}

} // namespace process
} // namespace astro
