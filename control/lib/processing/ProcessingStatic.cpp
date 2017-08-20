/*
 * ProcessingStatic.cpp -- implementation of static methods of the 
 *                         ProcesisngStep class
 *
 * (c) 2017 Prof Dr Andreas Mueller, Hochschule Rapperwil
 */
#include <AstroProcess.h>
#include <algorithm>
#include <includes.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <typeinfo>
#include <mutex>

namespace astro {
namespace process {

static std::recursive_mutex	process_mutex;
static int	_process_id = 0;

typedef std::map<int, ProcessingStepPtr>	stepmap_t;
static stepmap_t	allsteps;

/**
 * \brief Retreive the next available id
 */
int	ProcessingStep::newid() {
	std::unique_lock<std::recursive_mutex>	lock(process_mutex);
	return ++_process_id;
}

/**
 * \brief Add a processing step to the map
 */
void	ProcessingStep::remember(ProcessingStepPtr step) {
	std::unique_lock<std::recursive_mutex>	lock(process_mutex);
	if (allsteps.end() != allsteps.find(step->id())) {
		return;
	}
	allsteps.insert(std::make_pair(step->id(), step));
}

/**
 * \brief Find out whether a certain id is still in use
 */
bool	ProcessingStep::exists(int id) {
	std::unique_lock<std::recursive_mutex>	lock(process_mutex);
	return (allsteps.end() != allsteps.find(id));
}

/**
 * \brief Find a processing step given the id
 */
ProcessingStepPtr	ProcessingStep::byid(int id) {
	std::unique_lock<std::recursive_mutex>	lock(process_mutex);
	stepmap_t::const_iterator	i = allsteps.find(id);
	if (i != allsteps.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d", id);
		return i->second;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d not found", id);
	return ProcessingStepPtr(NULL);
}

/**
 * \brief Find out wheter a processing step is still in use
 */
bool	ProcessingStep::inuse(int id) {
	std::unique_lock<std::recursive_mutex>	lock(process_mutex);
	stepmap_t::const_iterator	i;
	i = std::find_if(allsteps.begin(), allsteps.end(),
		[id](const std::pair<int, ProcessingStepPtr>& p) {
			return p.second->hasPrecursor(id)
				|| p.second->hasSuccessor(id);
		}
	);
	return (i != allsteps.end());
}

/**
 * \brief Forget a certain measuring step (unless if it is still in use that is
 */
void	ProcessingStep::forget(int id) {
	std::unique_lock<std::recursive_mutex>	lock(process_mutex);
	if (inuse(id)) {
		std::string	msg = stringprintf("id %d still in use", id);
		throw std::runtime_error(msg);
	}
	stepmap_t::const_iterator	i = allsteps.find(id);
	if (i != allsteps.end()) {
		allsteps.erase(i);
	}
}

/**
 * \brief check the state of all the steps remembered by the system
 */
void	ProcessingStep::checkstate() {
	std::unique_lock<std::recursive_mutex>	lock(process_mutex);
	std::for_each(allsteps.begin(), allsteps.end(),
		[](const std::pair<int, ProcessingStepPtr>& p) mutable {
			ProcessingStepPtr	step = p.second;
			step->checkyourstate();
		}
	);
}

} // namespace process
} // namespace astro
