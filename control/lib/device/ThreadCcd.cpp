/*
 * ThreadCcd.cpp -- thread based implementation
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroExceptions.h>

namespace astro {
namespace camera {

/**
 * \brief Constructor for the ThreadCcd
 */
ThreadCcd::ThreadCcd(const CcdInfo& _info) : Ccd(_info) {
	_running = false;
}

/**
 * \brief main function to start the thread
 */
static void	treadccdmain(ThreadCcd *tc) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "treadccdmain starting");
	tc->run0();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "treadccdmain terminates");
}

/**
 * \brief run method
 */
void	ThreadCcd::run0() {
	try {
		this->run();
	} catch (const std::exception& x) {
		std::string msg = stringprintf("run() terminated by %s: %s",
			astro::demangle(typeid(x).name()).c_str(), x.what());
	} catch (...) {
	}
	_running = false;
}

/**
 * \brief startExposure
 *
 * This method starts the thread that is
 */
void	ThreadCcd::startExposure(const Exposure& exposure) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	// make sure we are in the right state
	if (exposureStatus() != CcdState::idle) {
		std::string	msg("not idle: start exposure in idle state");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadState(msg);
	}

	// find out whether there is a joinable thread
	if (_thread.joinable()) {
		_thread.join();
	}

	// prepare the exposure structure
	this->exposure = exposure;

	// start a thread
	state(CcdState::exposing);
	_running = true;
	_thread = std::thread(treadccdmain, this);
}

CcdState::State	ThreadCcd::exposureStatus() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	return state();
}

void ThreadCcd::cancelExposure() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	_running = false;
}

bool	ThreadCcd::wait() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	_condition.wait(lock);
	return (exposureStatus() == CcdState::exposed);
}


} // namespace camera
} // namespace astro
