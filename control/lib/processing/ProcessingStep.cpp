/*
 * ProcessingStep.cpp -- implement a network of dependent processing steps
 *
 * This was historically the first piece of the project that used lambdas
 * in an essential way
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperwil
 */
#include <AstroProcess.h>
#include <algorithm>
#include <includes.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <typeinfo>

using namespace astro::adapter;

namespace astro {
namespace process {

//////////////////////////////////////////////////////////////////////
// Construction and Destruction
//////////////////////////////////////////////////////////////////////
/**
 * \brief Create a new processing step
 */
ProcessingStep::ProcessingStep() {
	_id = newid();
	_status = idle;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new processing step %d created", _id);
}

/**
 * \brief Destroy the processing step
 */
ProcessingStep::~ProcessingStep() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroying %s @ %p, (id=%d, name=%s)",
		type_name().c_str(),  this, id(), name().c_str());
	// ensure we are neither precursor nor successor of any other step
	remove_me();
}

static std::string	get_typename(const ProcessingStep *step) {
	try {
		return demangle(typeid(*step).name());
	} catch (std::bad_typeid& x) {
		return stringprintf("(unknown [%s])", x.what());
	}
}

std::string	ProcessingStep::type_name() const {
	return get_typename(this);
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
bool	ProcessingStep::hasPrecursor(ProcessingStepPtr step) const {
	return hasPrecursor(step->id());
}

bool	ProcessingStep::hasPrecursor(int id) const {
	return _precursors.end() != std::find_if(_precursors.begin(),
		_precursors.end(),
		[id](int x) {
			return x == id;
		}
	);
}

bool	ProcessingStep::hasSuccessor(ProcessingStepPtr step) const {
	return hasSuccessor(step->id());
}

bool	ProcessingStep::hasSuccessor(int id) const {
	return _successors.end() != std::find_if(_successors.begin(),
		_successors.end(),
		[id](int x) {
			return x == id;
		}
	);
}

//////////////////////////////////////////////////////////////////////
// Dependency tracking
//////////////////////////////////////////////////////////////////////
/**
 * \brief add a precursor
 */
void	ProcessingStep::add_precursor(ProcessingStepPtr step) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add precursor %s @ %p",
		get_typename(&*step).c_str(),  &*step);
	// make sure this node is registered
	if (!exists(id())) {
		std::string	msg = stringprintf("%d not registered", id());
		throw std::runtime_error(msg);
	}
	remember(step);

	// check whether step is already a precursor
	if (hasPrecursor(step)) {
		return;
	}

	// mutually add links
	add_precursor(step->id());
	step->add_successor(id());
}

/**
 * \brief add a successor
 */
void	ProcessingStep::add_successor(ProcessingStepPtr step) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add successor %s @ %p",
		get_typename(&*step).c_str(),  &*step);
	// make sure this node is registered
	if (!exists(id())) {
		std::string	msg = stringprintf("%d not registered", id());
		throw std::runtime_error(msg);
	}
	remember(step);

	// check whether the step is already a successor
	if (hasSuccessor(step)) {
		return;
	}

	// mutually add links
	add_successor(step->id());
	step->add_precursor(id());
}

/**
 * \brief add a precursor
 */
void	ProcessingStep::remove_precursor(ProcessingStepPtr step) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove precursor %d", step->id());
	if (!hasPrecursor(step->id())) {
		return;
	}
	remove_precursor(step->id());
	step->remove_successor(id());
}

/**
 * \brief remove a successor
 */
void	ProcessingStep::remove_successor(ProcessingStepPtr step) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove successor %d", step->id());
	if (!hasSuccessor(step->id())) {
		return;
	}
	remove_successor(step->id());
	step->remove_precursor(id());
}

/**
 * \brief add a successor id
 */
void	ProcessingStep::add_successor(int id) {
	if (!exists(id)) {
		std::string	msg = stringprintf("id %d not found", id);
		throw std::runtime_error(msg);
	}
	if (hasSuccessor(id)) { return; }
	_successors.push_back(id);
}

/**
 * \brief add a precursor id
 */
void	ProcessingStep::add_precursor(int id) {
	if (!exists(id)) {
		std::string	msg = stringprintf("id %d not found", id);
		throw std::runtime_error(msg);
	}
	if (hasPrecursor(id)) { return; }
	_precursors.push_back(id);
}

/**
 * \brief Predicate class to find ids
 */
class	findid {
	int	_id;
public:
	findid(int id) : _id(id) { }
	bool	operator()(const int id) {
		return id == _id;
	}
};

/**
 * \brief remove a successor with a given id
 *
 * This method only changes the _successor list, it does not remove 
 */
void	ProcessingStep::remove_successor(int id) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove successor %d from %d", id, _id);
	steps::iterator	s = std::find_if(_successors.begin(), _successors.end(),
		findid(id));
	if (s != _successors.end()) {
		_successors.erase(s);
	}
}

/**
 * \brief remove a precursor with a given id
 */
void	ProcessingStep::remove_precursor(int id) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove precursor %d from %d", id, _id);
	steps::iterator	s = std::find_if(_precursors.begin(), _precursors.end(),
		findid(id));
	if (s != _precursors.end()) {
		_precursors.erase(s);
	}
}

/**
 * \brief Remove a processing step
 */
void	ProcessingStep::remove_me() {
	int	myid = _id;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove myself (%d) from linked nodes",
		myid);

	// remove me from precursors
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove %d from precursors", myid);
	std::for_each(_precursors.begin(), _precursors.end(),
		[myid](int precursorid) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "remove %d from %d",
				myid, precursorid);
			ProcessingStepPtr	pre = byid(precursorid);
			if (pre) {
				pre->remove_successor(myid);
			}
		}
	);

	// remove me from successors
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove %d successors", myid);
	std::for_each(_successors.begin(), _successors.end(),
		[myid](int successorid) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "remove %d from %d",
				myid, successorid);
			ProcessingStepPtr	suc = byid(successorid);
			if (suc) {
				suc->remove_precursor(myid);
			}
		}
	);
}

//////////////////////////////////////////////////////////////////////
// Processing
//////////////////////////////////////////////////////////////////////

/**
 * \brief Work on this step
 */
void	ProcessingStep::work(ProcessingThread *thread) {
	// ensure that we really are in state needswork, by checking all
	// precursors
	if (_status != needswork) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no work needed");
		return;
	}

	// set the status to working
	status(working);

	// signal to the calling thread that we have started up
	if (thread != NULL) {
		thread->started();
	}
	
	// if there is need for work, do the work
	status(do_work());
}

/**
 * \brief Dummy work method
 */
ProcessingStep::state	ProcessingStep::do_work() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "1 second dummy work");
	sleep(1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "dummy work done");
	return complete;
}

/**
 * \brief Cancellation
 *
 * Default imlementation does not do anything
 */
void	ProcessingStep::cancel() {
	return;
}

//////////////////////////////////////////////////////////////////////
// State management
//////////////////////////////////////////////////////////////////////
std::string	ProcessingStep::statename(state s) {
	switch (s) {
	case idle:	return std::string("idle");
	case needswork:	return std::string("needswork");
	case working:	return std::string("working");
	case complete:	return std::string("complete");
	}
	throw std::runtime_error("internal error: unknown state");
}

ProcessingStep::state	ProcessingStep::precursorstate() const {
	// if there is no precursor, then we can consider them all complete
	if (_precursors.size() == 0) {
		return complete;
	}

	// if there are any precursors, we have to check their minimum state
	int	minid = *std::min_element(_precursors.begin(),
		_precursors.end(),
		[](const int a, const int b) {
			return byid(a)->status() < byid(b)->status();
		}
	);
	state	minstate = byid(minid)->status();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "precursor state: %s",
		statename(minstate).c_str());
	return minstate;
}

/**
 * \brief verify the current state of the step
 *
 * Derived classes may want to override this method to ensure that the are
 * always in state idle as long as they are not fully configured. Maybe we
 * could remove this requirement by adding an additional state unconfigured
 * below the idle level.
 */
ProcessingStep::state	ProcessingStep::checkstate() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking state in %p", this);
	// find the smallest state that we should be in according to
	// our predecessors. We use a lambda comparator for this purpose
	state	minstate = precursorstate();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "lowest precursor: %s",
		statename(minstate).c_str());

	// if the state changes, signal to all our successors that we
	// have changed state, we again use a lambda for this
	state	newstate = minstate;
	if (minstate == complete) {
		if (_status == idle) {
			newstate = needswork;
		} else {
			newstate = _status;
		}
	} else {
		newstate = idle;
	}

	// if the state does not change, return (this terminates the recursion)
	if (newstate == status()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no state change");
		return status();
	}

	// change state, and tell successors to check their state too
	debug(LOG_DEBUG, DEBUG_LOG, 0, "change state to %s",
		statename(minstate).c_str());
	_status = newstate;
	std::for_each(_successors.begin(), _successors.end(),
		[](int stepid) {
			ProcessingStepPtr	step = byid(stepid);
			step->checkstate();
		}
	);

	// return the new state
	return status();
}

ProcessingStep::state	ProcessingStep::status(state newstate) {
	// first check the maximum state we can possible have
	state	pc = precursorstate();
	if (newstate > pc) {
		return _status;
	}
	if (_status == newstate) {
		return _status;
	}
	_status = newstate;
	std::for_each(_successors.begin(), _successors.end(),
		[](int successorid) {
			byid(successorid)->checkstate();
		}
	);
	return _status;
}

//////////////////////////////////////////////////////////////////////
// meta data access
//////////////////////////////////////////////////////////////////////
/**
 * \brief Find out whether the meta value is present
 */
bool	ProcessingStep::hasMetadata(const std::string& /* name */) const {
	return false;
}

/**
 * \brief Find the value of a meta data element
 */
astro::image::Metavalue	ProcessingStep::getMetadata(const std::string& name) const {
	std::string	msg = stringprintf("unknown extensions '%s'",
		name.c_str());
	throw std::runtime_error(msg);
}

} // namespace process
} // namespace astro
