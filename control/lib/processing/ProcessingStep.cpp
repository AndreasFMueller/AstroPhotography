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
ProcessingStep::ProcessingStep() : _barrier(2) {
	_id = newid();
	_status = idle;
	_when = 0;
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
	steps::const_iterator	s = std::find_if(_successors.begin(), _successors.end(),
		findid(id));
	if (s != _successors.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found successor %d in %d",
			id, _id);
		_successors.erase(s);
	}
}

/**
 * \brief remove a precursor with a given id
 */
void	ProcessingStep::remove_precursor(int id) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove precursor %d from %d", id, _id);
	steps::const_iterator	s = std::find_if(_precursors.begin(), _precursors.end(),
		findid(id));
	if (s != _precursors.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found precursor %d in %d",
			id, _id);
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
		[myid](int precursorid) mutable {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"remove %d from precursor %d",
				myid, precursorid);
			ProcessingStepPtr	pre = byid(precursorid);
			if (pre) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"removing %d from %d", myid, pre->id());
				pre->remove_successor(myid);
			}
		}
	);

	// remove me from successors
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove %d from successors", myid);
	std::for_each(_successors.begin(), _successors.end(),
		[myid](int successorid) mutable {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"remove %d from successor %d",
				myid, successorid);
			ProcessingStepPtr	suc = byid(successorid);
			if (suc) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"removing %d from %d", myid, suc->id());
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
void	ProcessingStep::work() {
	// ensure that we really are in state needswork, by checking all
	// precursors
	if (status() != needswork) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no work needed");
		return;
	}

	// set the status to working
	status(working);
	state	_resultstate = failed;

	// use the barrier to make sure the calling 
	//_barrier.await();

	// show what you are doing
	std::string	msg = stringprintf("id=%d start %s",
				_id, what().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
	if (verbose()) {
		std::cout << msg << std::endl;
	}
	Timer	timer;
	timer.start();

	// if there is need for work, do the work
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d calling %s::do_work()",
			id(), demangle(typeid(*this).name()).c_str());
		_resultstate = do_work();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d %s::do_work() completed: %s",
			id(), demangle(typeid(*this).name()).c_str(),
			ProcessingStep::statename(_resultstate).c_str());
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "processing step failed: %s",
			x.what());
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"processing step failed, unknown reason");
	}

	timer.end();
	msg = stringprintf("%d takes %.3fs", _id, timer.elapsed());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
	if (verbose()) {
		std::cout << msg << std::endl;
	}
	status(_resultstate);
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
	case failed:	return std::string("failed");
	}
	throw std::runtime_error("internal error: unknown state");
}

/**
 * \brief Find the state of the precursors
 */
ProcessingStep::state	ProcessingStep::precursorstate() const {
	// if there is no precursor, then we can consider them all complete
	if (_precursors.size() == 0) {
		throw std::logic_error("cannot query precursor state without precursors");
	}

	// if there are any precursors, we have to check their minimum state
	state	minstate = failed;
	steps::const_iterator	i;
	for (i = _precursors.begin(); i != _precursors.end(); i++) {
		state	newstatus = byid(*i)->status();
		if (newstatus < minstate) {
			minstate = newstatus;
			if (minstate == ProcessingStep::idle) {
				return minstate;
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"'%s' (%d) %s minimum precursor state for %d precursors: %s",
		_name.c_str(), _id, demangle(typeid(*this).name()).c_str(),
		_precursors.size(), statename(minstate).c_str());
	return minstate;
}

ProcessingStep::state	ProcessingStep::status(state newstate) {
	_status = newstate;
	return _status;
}

/**
 * \brief Give a list of dependencies that are not satisfied
 */
std::list<int>	ProcessingStep::unsatisfied_dependencies() {
	std::list<int>	result;
	steps::const_iterator	i;
	for (i = _precursors.begin(); i != _precursors.end(); i++) {
		if (byid(*i)->when() > when()) {
			result.push_back(*i);
		}
	}
	return result;
}

/**
 * \brief Compute the time
 *
 * By default the time is the maximum time of all precursors.
 */
time_t	ProcessingStep::when() const {
	// if this step has no precursors, then we return the time it was
	// last computed
	if (_precursors.size() == 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"step %d '%s' (%s) no precursors, when = %d",
			_id, _name.c_str(),
			demangle(typeid(*this).name()).c_str(), _when);
		return _when;
	}

	// look for the largest time in all the precursors. For all nodes
	// except for the file nodes this is the right dependency time
	time_t	maxtime = 0;
	ProcessingStep::steps::const_iterator	i;
	for (i = _precursors.begin(); i != _precursors.end(); i++) {
		ProcessingStepPtr	step = byid(*i);
		time_t	newtime = step->when();
		if (newtime > maxtime) {
			maxtime = newtime;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"step %d '%s' (%s) has %d prec, when = %d",
		_id, _name.c_str(), demangle(typeid(*this).name()).c_str(),
		_precursors.size(), maxtime);
	return maxtime;
}

/**
 * \brief default status query implementation
 */
ProcessingStep::state	ProcessingStep::status() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "find status of '%s' (%d) %s",
		_name.c_str(), _id,
		demangle(typeid(*this).name()).c_str());
		
	// if we have no precursors, then our own style decides
	if (0 == _precursors.size()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no precursors: status %s",
			statename(_status).c_str());
		return _status;
	}

	// if any precursor is in failed state, you are in failed state as well
	if (std::any_of(_precursors.begin(), _precursors.end(),
		[](int precursorid) -> bool {
			ProcessingStepPtr	precursor = byid(precursorid);
			bool	result = (ProcessingStep::failed
					== precursor->status());
			if (result) {
				std::string	name = demangle(
					typeid(*precursor).name());
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"step %s (%s) failed",
					precursor->name().c_str(),
					name.c_str());
			}
			return result;
		}
	)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "some precursors failed: failed");
		return ProcessingStep::failed;
	}

	// use the precursorstate
	switch (precursorstate()) {
	case idle:
	case needswork:
	case working:
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"not all precursors of '%s' (%d) %s are complete",
			_name.c_str(), _id,
			demangle(typeid(*this).name()).c_str());
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is idle",
				_name.c_str());
		return ProcessingStep::idle;
	case complete:
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"precursors of '%s' (%d) %s are all complete",
			_name.c_str(), _id,
			demangle(typeid(*this).name()).c_str());
		if (_status != complete) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s needs work",
				_name.c_str());
			return ProcessingStep::needswork;
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s complete",
				_name.c_str());
			return ProcessingStep::complete;
		}
	case failed:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s failed",
			_name.c_str());
		return ProcessingStep::failed;
	}
	throw std::runtime_error("cannot determine my status");
}

void	ProcessingStep::dumpSuccessors(std::ostream& out) const {
	std::for_each(_successors.begin(), _successors.end(),
		[&](int sid) {
			out << "        ";
			ProcessingStepPtr	s = byid(sid);
			out << s->name() << "(" << sid << ")";
			out << demangle(typeid(*s).name());
			out << std::endl;
		}
	);
}

void	ProcessingStep::dumpPrecursors(std::ostream& out) const {
	std::for_each(_precursors.begin(), _precursors.end(),
		[&](int sid) {
			out << "        ";
			ProcessingStepPtr	s = byid(sid);
			out << s->name() << "(" << sid << ")";
			out << demangle(typeid(*s).name());
			out << std::endl;
		}
	);
}

#if 0
std::string	ProcessingStep::srcfile(const std::string& file) const {
	return _srcpath->file(file);
}

std::string	ProcessingStep::dstfile(const std::string& file) const {
	return _dstpath->file(file);
}
#endif

} // namespace process
} // namespace astro
