/*
 * FocusProcessBase.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include <AstroIO.h>

namespace astro {
namespace focusing {

/**
 * \brief Construct a focus process
 */
FocusProcessBase::FocusProcessBase(unsigned long minposition,
	unsigned long maxposition)
	: FocusParameters(minposition, maxposition) {
	status(Focus::IDLE);
}

/**
 * \brief Construct a focus process from a parameter object
 */
FocusProcessBase::FocusProcessBase(const FocusParameters& parameters)
	: FocusParameters(parameters) {
	status(Focus::IDLE);
}

/**
 * \brief Destroy the Process object
 */
FocusProcessBase::~FocusProcessBase() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stopping the focus thread");
	_running = false;
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
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no callback to report state");
	}
}

/**
 * \brief Report a FocusElement to the installed callback
 */
void	FocusProcessBase::reportFocusElement(const FocusElement& fe) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reporting %s", fe.toString().c_str());
	if (_callback) {
		callback::CallbackDataPtr fecd(new FocusElementCallbackData(fe));
		(*_callback)(fecd);
		callback::CallbackDataPtr fcd(new FocusCallbackData(fe));
		(*_callback)(fcd);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no callback installed");
	}
}

/**
 * \brief Report a new image
 */
void	FocusProcessBase::reportImage(ImagePtr image) {
	if (_callback) {
		callback::CallbackDataPtr cd(new callback::ImageCallbackData(image));
		(*_callback)(cd);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no callback to report image");
	}
}

/**
 * \brief The measure part of the focus process
 */
bool	FocusProcessBase::measure0() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focus process measure0() starts");

	// collect the data
	unsigned long	delta = maxposition() - minposition();
	for (int step = 0; step <= steps(); step++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "focusing step %d", step);

		// move to the next position
		status(Focus::MOVING);
		reportState();
		unsigned long	pos = minposition() + step * delta / steps();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "step %d, position %lu",
			step, pos);
		moveto(pos);

		if (!_running) {
			goto failed;
		}

		// take an image
		status(Focus::MEASURING);
		reportState();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "taking an image");
		ImagePtr	image = get();

		// add Focusposition metadata
		image->setMetadata(io::FITSKeywords::meta(std::string("FOCUSPOS"), (long)pos));

		// send the image to the callback
		reportImage(image);

		// add the image and the position to the 
		FocusElementPtr	fe(new FocusElement(pos));
		fe->raw_image = image;
		_focus_elements->put(fe);

		if (!_running) {
			goto failed;
		}
	}
	status(Focus::MEASURED);
	reportState();
	_focus_elements->terminate();
	return true;

failed:
	debug(LOG_DEBUG, DEBUG_LOG, 0, "process cancelled");
	status(Focus::FAILED);
	_focus_elements->terminate();
	reportState();
	return false;
}

/**
 * \brief The evaluate part of the focus process
 */
bool	FocusProcessBase::evaluate0() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focus process evaluate0() starts");

	// prepare a Processor
	FocusProcessor	processor(method(), solver());
	processor.keep_images(true);	// we want to get rid of the images
					// ourselves

	FocusElementPtr	fe;
	do {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for next FE");
		fe = _focus_elements->get();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got an element %p", &*fe);

		if (!_running) {
			return false;
		}

		if (fe) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"processing new element %s",
				fe->toString().c_str());

			// process the element
			processor.process(*fe);

			// report the element
			reportFocusElement(*fe);
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "queue terminated");
		}
	} while (fe);

	// if we are not running, we should stop evaluating
	if (!_running) {
		status(Focus::FAILED);
		reportState();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "evaluation cancelled");
		return false;
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
		status(Focus::FAILED);
		reportState();
		std::string	msg = stringprintf("position %lu < %lu "
			"outside interval", position, minposition());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	if (position > maxposition()) {
		status(Focus::FAILED);
		reportState();
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
	status(Focus::FOCUSED);
	reportState();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focusing complete");
	return true;
}

/**
 * \brief The measure method wrapper
 *
 * This method takes care of catching exceptions during the focusing process
 */
void	FocusProcessBase::measure() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start measure thread");
	try {
		bool	completed = measure0();
		if (!completed) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"focus process was terminated");
		}
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("cannot focus: %s",
			x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		status(Focus::FAILED);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "measure thread terminates");
}

/**
 * \brief The evaluate() wrapper method
 */
void	FocusProcessBase::evaluate() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start evaluate thread");
	try {
		bool	completed = evaluate0();
		if (!completed) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"evaluate process was terminated");
		}
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("cannot evaluate: %s",
			x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		status(Focus::FAILED);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "evaluate thread terminates");
}

bool	FocusProcessBase::completed() const {
	return (_status == Focus::FOCUSED) || (_status == Focus::FAILED);
}

/**
 * \brief Trampoline function to launch into the measure thread of the proces
 */
static void	measure_launch(FocusProcessBase *process) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "measure thread launched");
	process->measure();
}

/**
 * \brief Trampoline function to launch into the evaluate thread of the proces
 */
static void	evaluate_launch(FocusProcessBase *process) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "evaluate thread launched");
	process->evaluate();
}

/**
 * \brief Start the focus process
 */
void	FocusProcessBase::start() {
	// make sure the current state is IDLE
	if ((Focus::IDLE != status()) && (Focus::FOCUSED != status())
		&& (Focus::FAILED != status())) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "process not idle");
		throw std::runtime_error("FocusProcess not IDLE/FOCUSED/FAILED");
	}

	// if we are in status FOCUSED or FAILED, we first have to wait
	// wo clean up the previous run
	if ((Focus::FOCUSED == status()) || (Focus::FAILED == status())) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for threads to complete");
		try {
			wait();
		} catch (...) { }
	}

	// (re)start the process
	_running = true;

	// prepare a queue 
	_focus_elements = FocusElementQueuePtr(new FocusElementQueue());

	// start the evaluate thread
	_evaluate_thread = std::thread(evaluate_launch, this);

	// start the measure thread
	status(Focus::MOVING);
	_measure_thread = std::thread(measure_launch, this);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "focus process threads started");
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for threads to complete");
	std::set<Focus::state_type>	states;
	states.insert(Focus::MEASURED);
	states.insert(Focus::FOCUSED);
	states.insert(Focus::FAILED);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for %d states", states.size());
	Focus::state_type	finalstate = _status.wait(states);
	if (_measure_thread.joinable()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "joining measure thread");
		_measure_thread.join();
	}
	if (_evaluate_thread.joinable()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "joining evaluate thread");
		_evaluate_thread.join();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "state %s reached",
		Focus::state2string(finalstate).c_str());
	if (Focus::FAILED == finalstate) {
		std::string	msg = stringprintf("focus process failed");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}

} // namespace focusing
} // namespace astro
