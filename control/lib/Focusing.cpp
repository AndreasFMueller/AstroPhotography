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

	thread = ThreadPtr(new astro::thread::Thread<FocusWork>(*work));
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

std::string	Focusing::name_of_method(focus_method m) {
	switch (m) {
	case FWHM:
		return std::string("fwhm");
	case MEASURE:
		return std::string("measure");
	}
	throw std::runtime_error("bad focus method");
}

std::string	Focusing::name_of_status(focus_status s) {
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

std::string	Focusing::name_of_mode(focus_mode m) {
	switch (m) {
	case ONE_SIDED:
		return std::string("one-sided");
		break;
	case TWO_SIDED:
		return std::string("two-sided");
		break;
	}
	throw std::runtime_error("bad focus mode");
}

Focusing::focus_mode	Focusing::mode_from_name(const std::string& name) {
	int	l = name.size();
	if (l == 0) {
		throw std::runtime_error("unknown mode");
	}
	if (name == std::string("one-sided").substr(0, l)) {
		return Focusing::ONE_SIDED;
	}
	if (name == std::string("two-sided").substr(0, l)) {
		return Focusing::TWO_SIDED;
	}
	throw std::runtime_error("unknown mode");
}

Focusing::focus_method	Focusing::method_from_name(const std::string& name) {
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
