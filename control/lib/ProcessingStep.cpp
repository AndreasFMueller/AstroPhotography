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
	_status = idle;
}

/**
 * \brief Destroy the processing step
 */
ProcessingStep::~ProcessingStep() {
	// ensure we are neither precursor nor successor of any other step
	remove_me();
}

//////////////////////////////////////////////////////////////////////
// Dependency tracking
//////////////////////////////////////////////////////////////////////
/**
 * \brief add a precursor
 */
void	ProcessingStep::add_precursor(ProcessingStep *step) {
	// don't add if already present
	if (_precursors.end()
		!= std::find(_precursors.begin(), _precursors.end(), step)) {
		return;
	}
	// add to the precursors vector
	_precursors.push_back(step);
	// tell the step that it has a new successor
	step->add_successor(this);
}

void	ProcessingStep::add_precursor(ProcessingStepPtr step) {
	add_precursor(&*step);
}

/**
 * \brief add a successor
 */
void	ProcessingStep::add_successor(ProcessingStep *step) {
	// don't add if already present
	if (_successors.end()
		!= std::find(_successors.begin(), _successors.end(), step)) {
		return;
	}
	// add to the successors vector
	_successors.push_back(step);
	// tell the new step that we are it's precursor
	step->add_precursor(this);
}

void	ProcessingStep::add_successor(ProcessingStepPtr step) {
	add_successor(&*step);
}

/**
 * \brief add a precursor
 */
void	ProcessingStep::remove_precursor(ProcessingStep *step) {
	// no need for work if not present
	steps::iterator	s = std::find(_precursors.begin(), _precursors.end(),
					step);
	if (_precursors.end() == s) {
		return;
	}
	// remove the precursor
	_precursors.erase(s);
	// tell the step that we are no longer a successor
	step->remove_successor(this);
}

void	ProcessingStep::remove_precursor(ProcessingStepPtr step) {
	remove_precursor(&*step);
}

/**
 * \brief remove a successor
 */
void	ProcessingStep::remove_successor(ProcessingStep *step) {
	// no need for work if not present
	steps::iterator	s = std::find(_successors.begin(), _successors.end(),
					step);
	if (_successors.end() == s) {
		return;
	}
	// remove from the successor list
	_successors.erase(s);
	// tell the successor that we are no longer a precursor
	step->remove_precursor(this);
}

void	ProcessingStep::remove_successor(ProcessingStepPtr step) {
	remove_successor(&*step);
}

/**
 * \brief Remove a processing step
 */
void	ProcessingStep::remove_me() {
	steps	stepvector = _precursors;
	std::for_each(stepvector.begin(), stepvector.end(),
		[this](steps::value_type& x) { x->remove_successor(this); }
	);
	stepvector = _successors;
	std::for_each(stepvector.begin(), stepvector.end(),
		[this](steps::value_type& x) { x->remove_precursor(this); }
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
	if (_status != needswork) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no work needed");
		return;
	}

	// set the status to working
	status(working);
	
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
	state	minstate = (*std::min_element(_precursors.begin(),
		_precursors.end(),
		[](const ProcessingStep *a, const ProcessingStep *b) {
			return a->status() < b->status();
		}
	))->status();
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "lowest precursor: %d", minstate);

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
		[](steps::value_type x) { x->checkstate(); });

	// return the new state
	return status();
}

ProcessingStep::state	ProcessingStep::status(state newstate) {
	state	pc = precursorstate();
	// first check the maximum state we can possible have
	if (newstate > pc) {
		return _status;
	}
	if (_status == newstate) {
		return _status;
	}
	_status = newstate;
	std::for_each(_successors.begin(), _successors.end(),
		[](steps::value_type x) { x->checkstate(); });
	return _status;
}

//////////////////////////////////////////////////////////////////////
// Preview access
//////////////////////////////////////////////////////////////////////
PreviewMonochromeAdapter	ProcessingStep::monochrome_preview() {
	return PreviewMonochromeAdapter(preview);
}

PreviewColorAdapter	ProcessingStep::color_preview() {
	return PreviewColorAdapter(preview);
}

//////////////////////////////////////////////////////////////////////
// Access to output images
//////////////////////////////////////////////////////////////////////
const ConstImageAdapter<double>&	ProcessingStep::out() const {
	throw std::runtime_error("not implemented");
}

bool	ProcessingStep::hasColor() const {
	return false;
}

const ConstImageAdapter<RGB<double> >&	ProcessingStep::out_color() const {
	throw std::runtime_error("not implemented");
}

} // namespace process
} // namespace astro
