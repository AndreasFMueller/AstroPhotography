/*
 * BasicGuideport.cpp -- basic guider port implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <BasicGuideport.h>
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
void	BasicGuideport::do_activate(uint8_t active) {
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
 * This function simply calls the run method of the guideport
 */
static void	basicguideport_main(BasicGuideport *guideport) {
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "thread started");
		guideport->run();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "thread ended");
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "guideport terminated by "
			"%s: %s", demangle_cstr(x), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "guideport terminated by "
			"unknown exception");
	}
}

/**
 * \brief The run method of the guider port
 */
void	BasicGuideport::run() {
	std::unique_lock<std::mutex>	lock(mtx);
	auto	interval = std::chrono::milliseconds(100000);

	// make sure the device also has 0
	try {
		this->do_activate(0);
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"do_activate(0x0) should never throw");
	}

	// start endless loop
	do {
		// what time do we have now?
		std::chrono::steady_clock::time_point now
			= std::chrono::steady_clock::now();

		// when do we imperatively have our next stop?
		std::chrono::steady_clock::time_point next = now + interval;

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
		try {
			this->do_activate(_active);
		} catch (...) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"do_activate(0x%1x) should never throw",
				_active);
		}

		// wait for signal or state change
		debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for notification");
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
BasicGuideport::BasicGuideport(const std::string& devicename)
	: astro::camera::GuidePort(devicename), _running(true), _active(0),
	  thread(basicguideport_main, this) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "BasicGuideport %s constructed",
		devicename.c_str());
}

/**
 * \brief Destroy the thread
 */
BasicGuideport::~BasicGuideport() {
	stop();
}

/**
 * \brief Return the currently active guider port signals
 */
uint8_t	BasicGuideport::active() {
	return _active;
}

/**
 * \brief Activate guider port pins for a given set of times
 */
void	BasicGuideport::activate(float raplus, float raminus,
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread notified");
}

#if 0
/**
 * \brief signal to the thread that initialization is complete
 *
 * This causes the thread to start processing activation commands
 */
void	BasicGuideport::start() {
	std::unique_lock<std::mutex>	lock(mtx);
	cond.notify_one();
}
#endif

/**
 * \brief Stop the thread
 *
 * The thread will exit when it has processed the notification
 */
void	BasicGuideport::stop() {
	{
		std::unique_lock<std::mutex>	lock(mtx);
		_running = false;
	}
	cond.notify_one();
	if (thread.joinable()) {
		thread.join();
	}
}

} // namespace camera
} // namespace astro
