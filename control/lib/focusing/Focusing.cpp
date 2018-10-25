/*
 * Focusing.cpp -- implementation of auto focusing
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include <FocusWork.h>
#include "MeasureEvaluator.h"
#include "FWHM2Evaluator.h"
#include "FocusSolvers.h"

using namespace astro::camera;

namespace astro {
namespace focusing {

/**
 * \brief Create 
 */
Focusing::Focusing(CcdPtr ccd, FocuserPtr focuser)
	: _ccd(ccd), _focuser(focuser) {
	_method = std::string("fwhm");
	_status = Focus::IDLE;
	work = NULL;
	_steps = 3;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create Focusing @ %p", this);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Focusing @ %p", this);
	if (NULL != thread) {
		if (thread->isrunning()) {
			throw std::runtime_error("already focusing, "
				"cancel first");
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start focus search between %d and %d",
		min, max);
	_status = Focus::IDLE;

	// create the focus work
	FocusWork	*work = NULL;
	if (method() == "BrennerOmni") {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "initialize Brenner");
		evaluator(FocusEvaluatorFactory::get(std::string("BrennerOmni")));
		solver(FocusSolverPtr(new BrennerSolver()));
		work = new FocusWork(*this);
	}
	if (method() == "fwhm") {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "initialize FWHM");
		// these fields are handled in the VCurveFocusWork
		//evaluator(FocusEvaluatorFactory::get(
		//	FocusEvaluatorFactory::FWHM));
		//solver(FocusSolverPtr(new AbsoluteValueSolver()));
		work = new VCurveFocusWork(*this);
	}
	if (method() == "measure") {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "initialize Measure");
		//evaluator(FocusEvaluatorFactory::get(
		//	FocusEvaluatorFactory::Measure));
		//solver(FocusSolverPtr(new AbsoluteValueSolver()));
		work = new MeasureFocusWork(*this);
	}
	work->min(min);
	work->max(max);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focusing interval: [%d,%d]",
		work->min(), work->max());

	// start a thread with this work
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting a thread");
	typedef astro::thread::ThreadPtr	ThreadPtr;

	thread = ThreadPtr(new astro::thread::Thread<FocusWork>(work));
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
