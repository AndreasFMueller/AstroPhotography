/*
 * FocusProcess.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>

namespace astro {
namespace focusing {

FocusProcess::state_type FocusProcess::string2state(const std::string& s) {
	if (s == "idle") {
		return FocusProcess::IDLE;
	}
	if (s == "moving") {
		return FocusProcess::MOVING;
	}
	if (s == "measuring") {
		return FocusProcess::MEASURING;
	}
	if (s == "focused") {
		return FocusProcess::FOCUSED;
	}
	if (s == "failed") {
		return FocusProcess::FAILED;
	}
	throw std::runtime_error("bad focus status");
}

std::string	FocusProcess::state2string(state_type s) {
	switch (s) {
	case IDLE:
		return std::string("idle");
		break;
	case MOVING:
		return std::string("moving");
		break;
	case MEASURING:
		return std::string("measuring");
		break;
	case FOCUSED:
		return std::string("focused");
		break;
	case FAILED:
		return std::string("failed");
		break;
	}
	throw std::runtime_error("bad focus status");
}

/**
 * \brief Construct a focus process
 */
FocusProcess::FocusProcess() {
	status(IDLE);
}

FocusProcess::~FocusProcess() {
}

/**
 * \brief The run method for the focus process
 */
void	FocusProcess::run0() {
	// XXX verify that all the data is consistent

	// prepare a Processor
	FocusProcessor	processor(method(), solver());
	processor.keep_images(true); // we want to get rid of the ourselves

	// collect the data
	unsigned long	delta = _maxposition - _minposition;
	for (int step = 0; step <= _steps; step++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "focusing step %d", step);

		// move to the next position
		status(MOVING);
		unsigned long	pos = _minposition + step * delta / _steps;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "step %d, position %lu",
			step, pos);
		moveto(pos);

		// take an image
		status(MEASURING);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "taking an image");
		ImagePtr	image = get(_exposure);

		// add the image and the position to the 
		FocusElement	fe(pos);
		fe.raw_image = image;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "processing the image");
		processor.process(fe);
	}

	// get the input date for the solver
	FocusItems	items = processor.output()->items();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d items for focus");

	// solving
	FocusSolverPtr	solverptr = FocusSolverFactory::get(solver());
	unsigned long	position = solverptr->position(items);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found focus position %lu", position);

	// make sure the position is in the interval
	if (position < _minposition) {
		std::string	msg = stringprintf("position %lu < %lu "
			"outside interval", position, _minposition);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	if (position > _maxposition) {
		std::string	msg = stringprintf("position %lu > %lu "
			"outside interval", position, _maxposition);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// use the position for focusing
	debug(LOG_DEBUG, DEBUG_LOG, 0, "move to the final focus position %lu",
		position);
	moveto(position);

	// now you can declare the 
	status(FOCUSED);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focusing complete");
}

/**
 * \brief The run method wrapper
 *
 * This method takes care of catching exceptions during the focusing process
 */
void	FocusProcess::run() {
	try {
		run0();
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("cannot focus: %s",
			x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		status(FAILED);
	}
}

static void	launch(FocusProcess *process) {
	process->run();
}

/**
 * \brief Start the focus process
 */
void	FocusProcess::start() {
	// make sure the current state is IDLE
	if (IDLE != status()) {
		throw std::runtime_error("FocusProcess not IDLE");
	}
	_running = true;

	// start the thread
	status(MOVING);
	_thread = std::thread(launch, this);
}

/**
 * \brief Stop the focus process
 */
void	FocusProcess::stop() {
	_running = false;
	wait();
}

/**
 * \brief Wait for termination of the process
 *
 * wait returns when state FOCUSED or FAILED are reached.
 */
void	FocusProcess::wait() {
	std::set<state_type>	states;
	states.insert(FOCUSED);
	states.insert(FAILED);
	_status.wait(states);
}

} // namespace focusing
} // namespace astro
