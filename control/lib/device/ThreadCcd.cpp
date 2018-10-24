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
 *
 * \param _info		the CcdInfo for this CCD
 */
ThreadCcd::ThreadCcd(const CcdInfo& _info) : Ccd(_info) {
	_running = false;
}

/**
 * \brief main function to start the thread
 *
 * Trampoline function to forward the thread to the run() method
 * of the ThreadCcd object supplied as the argument.
 *
 * \param tc	the ThreadCcd object containing the run method to execute
 */
static void	treadccdmain(ThreadCcd *tc) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "treadccdmain starting");
	tc->run0();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "treadccdmain terminates");
}

/**
 * \brief run method
 *
 * Protective wrapper around the run method to handle exceptions thrown
 * by the run method. This ensures that the run() method cannot crash
 * the whole system.
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
 * This method starts the thread that is doing the actual work of exposing.
 * This is what the overridden run() method is supposed to do
 *
 * \param exposure	the exposure parameters
 */
void	ThreadCcd::startExposure(const Exposure& exposure) {
	Ccd::startExposure(exposure);

	// find out whether there is a joinable thread. if so, join it so
	// that we can overwrite the thread with impunity
	if (_thread.joinable()) {
		_thread.join();
	}

	// remember the thread we have launched
	_running = true;
	_thread = std::thread(treadccdmain, this);
}

/**
 * \brief Get the exposure status
 *
 * Note that state changes should be done by the run method. If this is
 * not possible, this method must be overridden.
 */
CcdState::State	ThreadCcd::exposureStatus() {
	return state();
}

/**
 * \brief Cancel an exposure
 *
 * This is done by by setting the _running flag to false. The run() method
 * is expected to check this flag at suitable intervals and to take action
 * to cancel the exposure in the device (if that is at all possible). The
 * run() method is also responsible for setting the state to cancelling and
 * to idle when cancelling is completet.
 */
void ThreadCcd::cancelExposure() {
	_running = false;
}

} // namespace camera
} // namespace astro
