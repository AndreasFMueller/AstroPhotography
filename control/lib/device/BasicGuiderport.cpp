/*
 * BasicGuiderport.cpp -- basic guider port implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <BasicGuiderport.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <includes.h>
#include <stdexcept>
#include <AstroDevice.h>
#include <chrono>
#include <sstream>

namespace astro {
namespace camera {

/**
 * \brief Activate the ports
 *
 * This method must be overridden by derived classes actually implementing
 * a hardware guider port.
 */
void	BasicGuiderport::do_activate(uint8_t active) {
	std::string	statereport;
	if (active & RAPLUS) {
		statereport += "RA+ ";
	} else {
		statereport += "ra+ ";
	}
	if (active & RAMINUS) {
		statereport += "RA- ";
	} else {
		statereport += "ra- ";
	}
	if (active & DECPLUS) {
		statereport += "DEC+ ";
	} else {
		statereport += "dec+ ";
	}
	if (active & DECMINUS) {
		statereport += "DEC- ";
	} else {
		statereport += "dec- ";
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "activate: %s", statereport.c_str());
}

/**
 * \brief main function for the guider port thread
 *
 * This function simply calls the run method of the guiderport
 */
static void	basicguiderport_main(BasicGuiderport *guiderport) {
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "thread started");
		guiderport->run();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "thread ended");
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "guiderport terminated by "
			"%s: %s", demangle(typeid(x).name()).c_str(), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "guiderport terminated by "
			"unknown exception");
	}
}

/**
 * \brief The run method of the guider port
 */
void	BasicGuiderport::run() {
	_running = true;
	std::unique_lock<std::mutex>	lock(mtx);

	// wait on the condition variable for the start signal
	cond.wait(lock);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start signal received");

	// start endless loop
	do {
		// what time do we have now?
		std::chrono::steady_clock::time_point now
			= std::chrono::steady_clock::now();

		// when do we imperatively have our next stop?
		std::chrono::steady_clock::time_point next = now +
				std::chrono::milliseconds(1000);

		// set the active pins
		uint8_t	a = 0;
		if (now < nextchange[0]) {
			if (nextchange[0] > now) {
				next = std::min(nextchange[0], next);
			}
			a |= RAPLUS;
		}
		if (now < nextchange[1]) {
			if (nextchange[1] > now) {
				next = std::min(nextchange[1], next);
			}
			a |= RAMINUS;
		}
		if (now < nextchange[2]) {
			if (nextchange[2] > now) {
				next = std::min(nextchange[2], next);
			}
			a |= DECPLUS;
		}
		if (now < nextchange[3]) {
			if (nextchange[3] > now) {
				next = std::min(nextchange[3], next);
			}
			a |= DECMINUS;
		}
		_active = a;

		// really activate the output pins
		do_activate(_active);

		// wait for signal or state change
		cond.wait_until(lock, next);
	} while (_running);
}

/**
 * \brief Create a basic guider port
 *
 * The constructor also launches the guider port thread. Derived classes
 * must however use the start method to actually start the thread, as
 * is waiting in the run function for the initialization to complete.
 */
BasicGuiderport::BasicGuiderport(const std::string& devicename)
	: astro::camera::GuiderPort(devicename),
	  thread(basicguiderport_main, this) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "BasicGuiderport %s constructed",
		devicename.c_str());
}

/**
 * \brief Destroy the thread
 */
BasicGuiderport::~BasicGuiderport() {
	stop();
	thread.join();
}

/**
 * \brief Return the currently active guider port signals
 */
uint8_t	BasicGuiderport::active() {
	return _active;
}

/**
 * \brief Activate guider port pins for a given set of times
 */
void	BasicGuiderport::activate(float raplus, float raminus,
		float decplus, float decminus) {
	std::chrono::steady_clock::time_point now
		= std::chrono::steady_clock::now();
	long long	delta = (raplus * 1000);
	if (delta > 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "activate RA+ for %lldms",
			delta);
	}
	nextchange[0] = now + std::chrono::milliseconds(delta);

	delta = (raminus * 1000);
	if (delta > 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "activate RA- for %lldms",
			delta);
	}
	nextchange[1] = now + std::chrono::milliseconds(delta);

	delta = (decplus * 1000);
	if (delta > 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "activate DEC+ for %lldms",
			delta);
	}
	nextchange[2] = now + std::chrono::milliseconds(delta);

	delta = (decminus * 1000);
	if (delta > 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "activate DEC- for %lldms",
			delta);
	}
	nextchange[3] = now + std::chrono::milliseconds(delta);

	cond.notify_one();
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "thread notified");
}

/**
 * \brief signal to the thread that initialization is complete
 *
 * This causes the thread to start processing activation commands
 */
void	BasicGuiderport::start() {
	std::unique_lock<std::mutex>	lock(mtx);
	cond.notify_one();
}

/**
 * \brief Stop the thread
 *
 * The thread will exit when it has processed the notification
 */
void	BasicGuiderport::stop() {
	std::unique_lock<std::mutex>	lock(mtx);
	_running = false;
	cond.notify_one();
}

} // namespace camera
} // namespace astro
