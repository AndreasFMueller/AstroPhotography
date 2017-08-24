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

/**
 * \brief Auxiliary class for static stuff
 *
 * We put everything into this class to get control over the construction/
 * destruction sequence
 */
class ProcessingSteps {
	std::recursive_mutex	_process_mutex;
	int	_process_id;
	typedef std::map<int, ProcessingStepPtr>	stepmap_t;
	stepmap_t	_allsteps;
	bool	_verbose;
public:
	ProcessingSteps() {
		_process_id = 0;
		_verbose = false;
	}
	int	newid();
	void	remember(ProcessingStepPtr step);
	void	forget(int id);
	void	clear();
	ProcessingStepPtr	byid(int id);
	bool	inuse(int id);
	bool	exists(int id);
	void	checkstate();
	void	verbose(bool v);
	bool	verbose();
};

// static variables and initialization
static ProcessingSteps	*ps = NULL;
static std::once_flag	ps_flag;
static void	ps_init() {
	ps = new ProcessingSteps();
}

//////////////////////////////////////////////////////////////////////
// implementation of ProcessingSteps member methods
//////////////////////////////////////////////////////////////////////
/**
 * \brief Retreive the next available id
 */
int	ProcessingSteps::newid() {
	std::unique_lock<std::recursive_mutex>	lock(_process_mutex);
	return ++_process_id;
}

/**
 * \brief Add a processing step to the map
 */
void	ProcessingSteps::remember(ProcessingStepPtr step) {
	std::unique_lock<std::recursive_mutex>	lock(_process_mutex);
	if (_allsteps.end() != _allsteps.find(step->id())) {
		return;
	}
	_allsteps.insert(std::make_pair(step->id(), step));
}

/**
 * \brief Find out whether a certain id is still in use
 */
bool	ProcessingSteps::exists(int id) {
	std::unique_lock<std::recursive_mutex>	lock(_process_mutex);
	return (_allsteps.end() != _allsteps.find(id));
}

/**
 * \brief Find a processing step given the id
 */
ProcessingStepPtr	ProcessingSteps::byid(int id) {
	std::unique_lock<std::recursive_mutex>	lock(_process_mutex);
	stepmap_t::const_iterator	i = _allsteps.find(id);
	if (i != _allsteps.end()) {
		return i->second;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d not found", id);
	return ProcessingStepPtr(NULL);
}

/**
 * \brief Find out wheter a processing step is still in use
 */
bool	ProcessingSteps::inuse(int id) {
	std::unique_lock<std::recursive_mutex>	lock(_process_mutex);
	stepmap_t::const_iterator	i;
	i = std::find_if(_allsteps.begin(), _allsteps.end(),
		[id](const std::pair<int, ProcessingStepPtr>& p) {
			return p.second->hasPrecursor(id)
				|| p.second->hasSuccessor(id);
		}
	);
	return (i != _allsteps.end());
}

/**
 * \brief Forget a certain measuring step (unless if it is still in use that is
 */
void	ProcessingSteps::forget(int id) {
	std::unique_lock<std::recursive_mutex>	lock(_process_mutex);
	if (inuse(id)) {
		std::string	msg = stringprintf("id %d still in use", id);
		throw std::runtime_error(msg);
	}
	stepmap_t::const_iterator	i = _allsteps.find(id);
	if (i != _allsteps.end()) {
		_allsteps.erase(i);
	}
}

/**
 * \brief check the state of all the steps remembered by the system
 */
void	ProcessingSteps::checkstate() {
	std::unique_lock<std::recursive_mutex>	lock(_process_mutex);
	std::for_each(_allsteps.begin(), _allsteps.end(),
		[](const std::pair<int, ProcessingStepPtr>& p) mutable {
			ProcessingStepPtr	step = p.second;
			//step->checkyourstate();
		}
	);
}

/**
 * \brief check the verbose flat
 */
bool	ProcessingSteps::verbose() {
	return _verbose;
}

/**
 * \brief change the global singleton verbose flag
 */
void	ProcessingSteps::verbose(bool v) {
	_verbose = v;
}

void	ProcessingSteps::clear() {
	_allsteps.clear();
}

//////////////////////////////////////////////////////////////////////
// Implementation of static ProcessingStep (without s) methods
//////////////////////////////////////////////////////////////////////
int	ProcessingStep::newid() {
	std::call_once(ps_flag, ps_init);
	if (ps) {
		return ps->newid();
	}
	return -1;
}

void	ProcessingStep::remember(ProcessingStepPtr step) {
	std::call_once(ps_flag, ps_init);
	if (ps) {
		ps->remember(step);
	}
}

void	ProcessingStep::forget(int id) {
	std::call_once(ps_flag, ps_init);
	if (ps) {
		ps->forget(id);
	}
}

void	ProcessingStep::clear() {
	std::call_once(ps_flag, ps_init);
	if (ps) {
		ps->clear();
	}
}

ProcessingStepPtr	ProcessingStep::byid(int id) {
	std::call_once(ps_flag, ps_init);
	if (ps) {
		return ps->byid(id);
	}
	return ProcessingStepPtr(NULL);
}

bool	ProcessingStep::inuse(int id) {
	std::call_once(ps_flag, ps_init);
	if (ps) {
		return ps->inuse(id);
	}
	return false;
}

bool	ProcessingStep::exists(int id) {
	std::call_once(ps_flag, ps_init);
	if (ps) {
		return ps->exists(id);
	}
	return false;
}

void	ProcessingStep::checkstate() {
	std::call_once(ps_flag, ps_init);
	if (ps) {
		ps->checkstate();
	}
}

bool	ProcessingStep::verbose() {
	std::call_once(ps_flag, ps_init);
	if (ps) {
		return ps->verbose();
	}
	return false;
}

void	ProcessingStep::verbose(bool v) {
	std::call_once(ps_flag, ps_init);
	if (ps) {
		ps->verbose(v);
	}
}

} // namespace process
} // namespace astro
