/*
 * Focusing.cpp -- implementation of auto focusing
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include <FocusWork.h>

using namespace astro::camera;

namespace astro {
namespace focusing {

/**
 * \brief Create 
 */
Focusing::Focusing(CcdPtr ccd, FocuserPtr focuser)
	: _ccd(ccd), _focuser(focuser) {
	_mode = TWO_SIDED;
	_status = IDLE;
	work = NULL;
	_steps = 3;
}

/**
 * \brief destroy the Focusing object
 *
 * If the thread is still running, it must be stopped
 */
Focusing::~Focusing() {
	if (thread) {
		thread->stop();
		thread->wait(1);
	}
	if (work) {
		delete work;
	}
}

/**
 * \brief Start the focusing process in a given interval
 */
void	Focusing::start(int min, int max) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start focus search between %d and %d",
		min, max);
	_status = IDLE;

	// create the focus work
	work = new FocusWork(*this);
	work->ccd(ccd());
	work->focuser(focuser());
	work->exposure(exposure());
	work->min(min);
	work->max(max);
	work->steps(steps());
	work->callback(callback());
	work->evaluator(evaluator());

	// start a thread with this work
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting a thread");

	thread = astro::thread::ThreadPtr(new astro::thread::Thread<FocusWork>(*work));
	thread->start();

	// that's it
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focusing thread started");
}

/**
 * \brief Cancel the focusing process
 */
void	Focusing::cancel() {
	if (thread) {
		thread->stop();
	}
}

} // namespace focusing
} // namespace astro
