/*
 * FocusProcessBase.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>

namespace astro {
namespace focusing {

/**
 * \brief Construct a focus process
 */
FocusProcessBase::FocusProcessBase(unsigned long minposition,
	unsigned long maxposition)
	: FocusParameters(minposition, maxposition) {
	status(IDLE);
}

/**
 * \brief Destroy the Process object
 */
FocusProcessBase::~FocusProcessBase() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stopping the focus thread");
	if (_running) {
		stop();
	}
	wait();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focus thread completed");
}

/**
 * \brief Report the state to the callback
 */
void	FocusProcessBase::reportState() {
	if (_callback) {
		FocusCallbackState	*f = new FocusCallbackState(status());
		callback::CallbackDataPtr	cd(f);
		(*_callback)(cd);
	}
}

/**
 * \brief Report a FocusElement to the installed callback
 */
void	FocusProcessBase::reportFocusElement(const FocusElement& fe) {
	if (_callback) {
		callback::CallbackDataPtr	fcd(new FocusCallbackData(fe));
		(*_callback)(fcd);
	}
}

/**
 * \brief Report a new image
 */
void	FocusProcessBase::reportImage(ImagePtr image) {
	if (_callback) {
		callback::CallbackDataPtr cd(new callback::ImageCallbackData(image));
		(*_callback)(cd);
	}
}

/**
 * \brief The run method for the focus process
 */
bool	FocusProcessBase::run0() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focus process run0() starts");

	// prepare a Processor
	FocusProcessor	processor(method(), solver());
	processor.keep_images(true); // we want to get rid of the ourselves

	// collect the data
	unsigned long	delta = maxposition() - minposition();
	for (int step = 0; step <= steps(); step++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "focusing step %d", step);

		// move to the next position
		status(MOVING);
		reportState();
		unsigned long	pos = minposition() + step * delta / steps();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "step %d, position %lu",
			step, pos);
		moveto(pos);

		if (!_running) {
			status(FAILED);
			reportState();
			return false;
		}

		// take an image
		status(MEASURING);
		reportState();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "taking an image");
		ImagePtr	image = get();

		reportImage(image);

		// add the image and the position to the 
		FocusElement	fe(pos);
		fe.raw_image = image;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "processing the image");
		processor.process(fe);

		// report the element
		reportFocusElement(fe);

		if (!_running) {
			status(FAILED);
			reportState();
			return false;
		}
	}

	// get the input date for the solver
	FocusItems	items = processor.output()->items();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d items for focus");

	// solving
	FocusSolverPtr	solverptr = FocusSolverFactory::get(solver());
	unsigned long	position = solverptr->position(items);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found focus position %lu", position);

	// make sure the position is in the interval
	if (position < minposition()) {
		std::string	msg = stringprintf("position %lu < %lu "
			"outside interval", position, minposition());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	if (position > maxposition()) {
		std::string	msg = stringprintf("position %lu > %lu "
			"outside interval", position, maxposition());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// use the position for focusing
	debug(LOG_DEBUG, DEBUG_LOG, 0, "move to the final focus position %lu",
		position);
	moveto(position);

	// now you can declare the 
	status(FOCUSED);
	reportState();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focusing complete");
	return true;
}

/**
 * \brief The run method wrapper
 *
 * This method takes care of catching exceptions during the focusing process
 */
void	FocusProcessBase::run() {
	try {
		bool	completed = run0();
		if (!completed) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"focus process was terminated");
		}
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("cannot focus: %s",
			x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		status(FAILED);
	}
}

/**
 * \brief Trampoline function to launch into the run method of the proces
 */
static void	launch(FocusProcessBase *process) {
	process->run();
}

/**
 * \brief Start the focus process
 */
void	FocusProcessBase::start() {
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
void	FocusProcessBase::stop() {
	_running = false;
	wait();
}

/**
 * \brief Wait for termination of the process
 *
 * wait returns when state FOCUSED or FAILED are reached.
 */
void	FocusProcessBase::wait() {
	std::set<state_type>	states;
	states.insert(FOCUSED);
	states.insert(FAILED);
	state_type	finalstate = _status.wait(states);
	if (FAILED == finalstate) {
		std::string	msg = stringprintf("focus process failed");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}

} // namespace focusing
} // namespace astro
