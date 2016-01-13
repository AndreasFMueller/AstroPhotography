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
	_method = FWHM;
	_status = IDLE;
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
	_status = IDLE;

	// create the focus work
	FocusWork	*work;
	switch (method()) {
	case Focusing::FWHM:
		work = new VCurveFocusWork(*this);
		break;
	case Focusing::MEASURE:
		work = new MeasureFocusWork(*this);
		break;
	}
	work->ccd(ccd());
	work->focuser(focuser());
	work->exposure(exposure());
	work->min(min);
	work->max(max);
	work->steps(steps());
	work->callback(callback());

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

std::string	Focusing::method2string(method_type m) {
	switch (m) {
	case FWHM:
		return std::string("fwhm");
	case MEASURE:
		return std::string("measure");
	}
	throw std::runtime_error("bad focus method");
}

std::string	Focusing::state2string(state_type s) {
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

Focusing::state_type	Focusing::string2state(const std::string& s) {
	if (s == "idle") {
		return Focusing::IDLE;
	}
	if (s == "moving") {
		return Focusing::MOVING;
	}
	if (s == "measuring") {
		return Focusing::MEASURING;
	}
	if (s == "focused") {
		return Focusing::FOCUSED;
	}
	if (s == "failed") {
		return Focusing::FAILED;
	}
	throw std::runtime_error("bad focus status");
}

Focusing::method_type	Focusing::string2method(const std::string& name) {
	int	l = name.size();
	if (l == 0) {
		throw std::runtime_error("unknown method");
	}
	if (name == std::string("fwhm").substr(0, l)) {
		return Focusing::FWHM;
	}
	if (name == std::string("measure").substr(0, l)) {
		return Focusing::MEASURE;
	}
	throw std::runtime_error("unknown method");
}

} // namespace focusing
} // namespace astro
