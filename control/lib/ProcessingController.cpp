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
		ProcessingThreadPtr step) {
	if (steps.find(name) != steps.end()) {
		throw std::runtime_error("duplicate processing step name");
	}
	steps.insert(std::make_pair(name, step));
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
std::string	ProcessingController::name(ProcessingThreadPtr step) {
	stepmap::iterator	s = std::find_if(steps.begin(), steps.end(),
		[step](stepmap::value_type& v) {
			return (v.second == step);
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
ProcessingThreadPtr	ProcessingController::find(const std::string& name) {
	stepmap::iterator	i = steps.find(name);
	if (i == steps.end()) {
		throw std::runtime_error("step named " + name + " not found");
	}
	return i->second;
}

//////////////////////////////////////////////////////////////////////
// Successors and predecessors
//////////////////////////////////////////////////////////////////////
void	ProcessingController::add_precursor(const std::string& target_name,
		const std::string& precursor_name) {
	find(target_name)->step()->add_precursor(find(precursor_name)->step());
}

void	ProcessingController::add_successor(const std::string& target_name,
		const std::string& successor_name) {
	find(target_name)->step()->add_successor(find(successor_name)->step());
}

void	ProcessingController::remove_precursor(const std::string& target_name,
		const std::string& precursor_name) {
	find(target_name)->step()->remove_precursor(find(precursor_name)->step());
}

void	ProcessingController::remove_successor(const std::string& target_name,
		const std::string& successor_name) {
	find(target_name)->step()->remove_successor(find(successor_name)->step());
}

} // namespace process
} // namespace astro
